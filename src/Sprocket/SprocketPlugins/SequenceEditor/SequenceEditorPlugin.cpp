/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SequenceEditor.h"
#include "SequenceMetaTypes.h"
#include "SequencePlotWidget.h"
#include "SequenceSerializer.h"
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/Icon/SprocketIcons.h>
#include <Sprocket/Entity/EntityChooserDialogFactory.h>
#include <Sprocket/Scenario/ScenarioObject.h>
#include <Sprocket/Scenario/ScenarioSelectionModel.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Sequence/SequenceController.h>
#include <SkyboltEngine/Sequence/EntityStateSequenceController.h>
#include <SkyboltEngine/Sequence/JulianDateSequenceController.h>

#include <QMainWindow>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>

using namespace skybolt;

using SequenceObject = ScenarioObjectT<StateSequenceControllerPtr>;

static void addItem(Registry<ScenarioObject>& registry, const StateSequenceControllerPtr& controller, const std::string& name)
{
	QIcon icon = getSprocketIcon(SprocketIcon::Sequence);
	registry.add(std::make_shared<SequenceObject>(name, icon, controller));
}

static StateSequenceControllerPtr toSequenceController(const SequenceObject& item)
{
	auto sequenceItem = static_cast<const SequenceObject*>(&item);
	return sequenceItem->data;
}

static void readSequences(Registry<ScenarioObject>& registry, const nlohmann::json& json, Scenario* scenario)
{
	assert(scenario);

	for (const auto& i : json.items())
	{
		StateSequenceControllerPtr controller = readSequenceController(i.value(), scenario);
		addItem(registry, controller, i.key());
	}
}

static nlohmann::json writeSequences(const Registry<ScenarioObject>& registry)
{
	nlohmann::json json;
	for (const auto& item : registry.getItems())
	{
		json[item->getName()] = writeSequenceController(*toSequenceController(*dynamic_cast<SequenceObject*>(item.get())));
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
		mEngineRoot(config.engineRoot)
	{
		assert(mEngineRoot);
		mConnections.push_back(mEngineRoot->scenario->timeSource.timeChanged.connect([this](double time) {
			setTime(time);
		}));

		QIcon icon = getSprocketIcon(SprocketIcon::Sequence);

		{
			auto type = std::make_shared<ScenarioObjectType>();
			type->name = "Sequence";
			type->category = "Sequence";
			type->templateNames = {"Entity", "DateTime"};
			type->objectCreator = [this, icon](const std::string& name, const std::string& subType) {
				StateSequenceControllerPtr sequence;
				if (subType == "Entity")
				{
					sequence = std::make_shared<EntityStateSequenceController>(std::make_shared<EntityStateSequence>());
				}
				else if (subType == "DateTime")
				{
					sequence = std::make_shared<JulianDateSequenceController>(std::make_shared<DoubleStateSequence>(), mEngineRoot->scenario.get());
				}
				assert(sequence);
				auto item = std::make_shared<SequenceObject>(name, icon, sequence);
				mSequenceObjectRegistry->add(item);
				return item;
			};
			type->isObjectRemovable = [] (const ScenarioObject& item) { return true; };
			type->objectRemover = [this](const ScenarioObject& item) {
				mSequenceObjectRegistry->remove(&item);
			};
			type->objectRegistry = mSequenceObjectRegistry;

			mScenarioObjectTypes[std::type_index(typeid(SequenceObject))] = type;
		}


		mSequencePlotWidget = new SequencePlotWidget(&mEngineRoot->scenario->timeSource, config.mainWindow);
		mToolWindow.name = "Curve Editor";
		mToolWindow.widget = mSequencePlotWidget;

		mFactoryMap[qMetaTypeId<StateSequenceControllerPtr>()] = [this](QtProperty* property, QWidget* parent) {
			auto sequence = property->value.value<StateSequenceControllerPtr>();
			SequenceEditorConfig config;
			config.controller = sequence;
			config.entityChooserDialogFactory = std::make_shared<EntityChooserDialogFactory>(&mEngineRoot->scenario->world);
			config.timeSource = &mEngineRoot->scenario->timeSource;
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

		QObject::connect(config.selectionModel, &ScenarioSelectionModel::selectionChanged, [this] (const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected) {
			selectionChanged(selected);
		});
	}

	~SequenceEditorPlugin() override = default;

	void resetProject() override
	{
		mSequenceObjectRegistry->clear();
		mRecordingControllers.clear();
	}

	void readProject(const nlohmann::json& json) override
	{
		ifChildExists(json, "sequences", [&] (const nlohmann::json& child) {
			readSequences(*mSequenceObjectRegistry, child, mEngineRoot->scenario.get());
		});
	}

	void writeProject(nlohmann::json& json)
	{
		json["sequences"] = writeSequences(*mSequenceObjectRegistry);
	}

	void setTime(double time)
	{
		for (const auto& item : mSequenceObjectRegistry->getItems())
		{
			const auto& sequence = toSequenceController(*dynamic_cast<SequenceObject*>(item.get()));

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
	
	PropertyModelFactoryMap getPropertyModelFactories() const override
	{
		return { { typeid(SequenceObject), [] (const ScenarioObject& item) {
			auto sequenceItem = dynamic_cast<const SequenceObject*>(&item);
			QtPropertyPtr property = createQtProperty("Sequence", QVariant::fromValue(sequenceItem->data));
			return std::make_shared<PropertiesModel>(std::vector<QtPropertyPtr>({ property }));
		}}};
	}

	PropertyEditorWidgetFactoryMap getPropertyEditorWidgetFactories() const override
	{
		return mFactoryMap;
	}

	ScenarioObjectTypeMap getSceneObjectTypes() const override
	{
		return mScenarioObjectTypes;
	}

	void selectionChanged(const SelectedScenarioObjects& selection)
	{
		if (auto sequenceItem = getFirstSelectedScenarioObjectOfType<SequenceObject>(selection))
		{
			mSequencePlotWidget->setSequenceController(sequenceItem->data);
		}
	}

	std::vector<ToolWindow> getToolWindows() override
	{
		return { mToolWindow };
	}

private:
	UiControllerPtr mUiController;
	ScenarioObjectRegistryPtr mSequenceObjectRegistry = std::make_shared<ScenarioObjectRegistry>();
	ScenarioObjectTypeMap mScenarioObjectTypes;
	ToolWindow mToolWindow;
	EngineRoot* mEngineRoot;
	std::vector<boost::signals2::scoped_connection> mConnections;
	SequencePlotWidget* mSequencePlotWidget;
	PropertyEditorWidgetFactoryMap mFactoryMap;
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
