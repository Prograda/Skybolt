/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SequencePropertiesModel.h"
#include "SequenceEditor.h"
#include "SequencePlotWidget.h"
#include "SequenceSerializer.h"
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/IconFactory.h>
#include <Sprocket/JsonHelpers.h>
#include <Sprocket/Entity/EntityChooserDialogFactory.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Sequence/SequenceController.h>
#include <SkyboltEngine/Sequence/EntityStateSequenceController.h>
#include <SkyboltEngine/Sequence/JulianDateSequenceController.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>

using namespace skybolt;

using SequenceTreeItem = TreeItemT<StateSequenceControllerPtr>;

static void addItem(Registry<TreeItem>& registry, const StateSequenceControllerPtr& controller, const QString& name)
{
	QIcon icon = getDefaultIconFactory().createIcon(IconFactory::Icon::Sequence);
	registry.add(std::make_shared<SequenceTreeItem>(icon, name, controller));
}

static StateSequenceControllerPtr toSequenceController(const TreeItem& item)
{
	auto sequenceItem = static_cast<const SequenceTreeItem*>(&item);
	return sequenceItem->data;
}

static void readSequences(Registry<TreeItem>& registry, const QJsonValue& value, const sim::World& world, Scenario* scenario)
{
	assert(!value.isUndefined());
	assert(scenario);

	QJsonObject json = value.toObject();
	for (const auto& name : json.keys())
	{
		StateSequenceControllerPtr controller = readSequenceController(json[name].toObject(), world, scenario);
		addItem(registry, controller, name);
	}
}

static QJsonObject writeSequences(const Registry<TreeItem>& registry)
{
	QJsonObject json;
	for (const auto& item : registry.getItems())
	{
		json[QString::fromStdString(item->getName())] = writeSequenceController(*toSequenceController(*item));
	}
	return json;
}

class SequenceRecorder
{
public:
	SequenceRecorder(const StateSequenceControllerPtr& controller) :
		mController(controller)
	{
		// Delete existing sequence
		mController->getSequence()->clear();
	}

	void setTime(double time)
	{
		double keyFrameInterval = 0.5;
		bool captureFrame = !mPreviousKeyframeTime || (time - *mPreviousKeyframeTime > keyFrameInterval);
		if (captureFrame)
		{
			auto state = mController->getState();
			auto sequence = mController->getSequence();
			if (state)
			{
				sequence->addItemAtTime(*state, time);
			}
			mPreviousKeyframeTime = time;
		}
	}

private:
	StateSequenceControllerPtr mController;
	std::optional<double> mPreviousKeyframeTime;
};

class SequenceEditorPlugin : public EditorPlugin
{
public:
	SequenceEditorPlugin(const EditorPluginConfig& config) :
		mUiController(config.uiController),
		mSequenceTreeItemRegistry(new Registry<TreeItem>()),
		mEngineRoot(config.engineRoot)
	{
		assert(mEngineRoot);
		mConnections.push_back(mEngineRoot->scenario.timeSource.timeChanged.connect([this](double time) {
			setTime(time);
		}));

		QIcon icon = getDefaultIconFactory().createIcon(IconFactory::Icon::Sequence);

		{
			TreeItemType type(typeid(SequenceTreeItem));
			type.name = "Sequence";
			type.subTypes = {"Entity", "DateTime"};
			type.itemCreator = [this, icon](const std::string& name, const std::string& subType) {
				StateSequenceControllerPtr sequence;
				if (subType == "Entity")
				{
					sequence = std::make_shared<EntityStateSequenceController>(std::make_shared<EntityStateSequence>());
				}
				else if (subType == "DateTime")
				{
					sequence = std::make_shared<JulianDateSequenceController>(std::make_shared<DoubleStateSequence>(), &mEngineRoot->scenario);
				}
				assert(sequence);
				mSequenceTreeItemRegistry->add(std::make_shared<SequenceTreeItem>(icon, QString::fromStdString(name), sequence));
			};
			type.itemDeleter = [this](TreeItem* item) {
				mSequenceTreeItemRegistry->remove(item);
			};
			type.itemRegistry = mSequenceTreeItemRegistry;

			mTreeItemTypes.push_back(type);
		}


		mSequencePlotWidget = new SequencePlotWidget(&mEngineRoot->scenario.timeSource);
		mToolWindow.name = "Curve Editor";
		mToolWindow.widget = mSequencePlotWidget;
	}

	~SequenceEditorPlugin()
	{
	}

	void clearProject()
	{
		mSequenceTreeItemRegistry->clear();
		mRecordingControllers.clear();
	}

	void loadProject(const QJsonObject& json)
	{
		QJsonValue value = json["sequences"];
		if (!value.isUndefined())
		{
			readSequences(*mSequenceTreeItemRegistry, value, *mEngineRoot->simWorld, &mEngineRoot->scenario);
		}
	}

	void saveProject(QJsonObject& json)
	{
		json["sequences"] = writeSequences(*mSequenceTreeItemRegistry);
	}

	void setTime(double time)
	{
		for (const auto& item : mSequenceTreeItemRegistry->getItems())
		{
			auto& sequence = toSequenceController(*item);

			auto i = mRecordingControllers.find(sequence);
			if (i == mRecordingControllers.end()) // playing
			{
				sequence->setTime(time);
			}
			else // recording
			{
				i->second->setTime(time);
			}
		}
	}
	
	std::vector<TreeItemType> getTreeItemTypes() override
	{
		return mTreeItemTypes;
	}

	PropertyEditorWidgetFactoryMap getPropertyEditorWidgetFactories() override
	{
		PropertyEditorWidgetFactoryMap factoryMap;
		factoryMap[typeid(SequenceProperty)] = [this](QtProperty& property) {
			auto sequence = static_cast<SequenceProperty*>(&property)->sequence;
			SequenceEditorConfig config;
			config.controller = sequence;
			config.entityChooserDialogFactory = std::make_shared<EntityChooserDialogFactory>(mEngineRoot->simWorld.get());
			config.timeSource = &mEngineRoot->scenario.timeSource;
			config.sequenceRecorder = [this, sequence, timeSource = config.timeSource](bool enabled) {
				if (enabled)
				{
					if (mRecordingControllers.find(sequence) == mRecordingControllers.end())
					{
						mRecordingControllers[sequence] = std::make_shared<SequenceRecorder>(sequence);
					}
					timeSource->setState(TimeSource::StatePlaying);
				}
				else
				{
					mRecordingControllers.erase(sequence);
					timeSource->setState(TimeSource::StateStopped);
				}
			};
			config.isRecording = (mRecordingControllers.find(sequence) != mRecordingControllers.end());
			config.parent = nullptr;
			return new SequenceEditor(config);
		};
		return factoryMap;
	}

	void explorerSelectionChanged(const TreeItem& item) override
	{
		if (auto sequenceItem = dynamic_cast<const SequenceTreeItem*>(&item))
		{
			mUiController->propertiesModelSetter(std::make_shared<SequencePropertiesModel>(sequenceItem->data));
			mSequencePlotWidget->setSequenceController(sequenceItem->data);
		}
	}

	std::vector<ToolWindow> getToolWindows() override
	{
		return { mToolWindow };
	}

private:
	UiControllerPtr mUiController;
	std::shared_ptr<Registry<TreeItem>> mSequenceTreeItemRegistry;
	std::vector<TreeItemType> mTreeItemTypes;
	ToolWindow mToolWindow;
	EngineRoot* mEngineRoot;
	std::vector<boost::signals2::scoped_connection> mConnections;
	SequencePlotWidget* mSequencePlotWidget;
	std::map<StateSequenceControllerPtr, std::shared_ptr<SequenceRecorder>> mRecordingControllers;
};

namespace plugins {

	std::shared_ptr<EditorPlugin> createEditorPlugin(const EditorPluginConfig& config)
	{
		return std::make_shared<SequenceEditorPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEditorPlugin,
		createEditorPlugin
	)
}
