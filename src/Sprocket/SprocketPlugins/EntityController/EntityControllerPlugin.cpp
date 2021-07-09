/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Sprocket/EditorPlugin.h>
#include <Sprocket/QtHelpers.h>

#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Input/InputPlatform.h>
#include <SkyboltCommon/VectorUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <assert.h>
#include <deque>
#include <memory>
#include <sstream>

using namespace skybolt;

typedef std::function<glm::vec2()> Vec2Getter;
typedef std::function<void(const glm::vec2&)> Vec2Setter;

class MouseXYAxisWidget : public QWidget
{
public:
	MouseXYAxisWidget(const Vec2Getter& getter, const Vec2Setter& setter, QWidget* parent = nullptr) :
		QWidget(parent),
		mGetter(getter),
		mSetter(setter)
	{
		setBackgroundRole(QPalette::Base);
		setAutoFillBackground(true);
	}

	void setResetOnReleaseValue(const glm::vec2& v)
	{
		mResetOnReleaseValue = v;
	}

private:
	void paintEvent(QPaintEvent* event) override
	{
		static const int crossRadiusPixels = 5;

		auto pos = mGetter() * 0.5f + 0.5f;

		int x = pos.x * width();
		int y = pos.y * height();

		QPen pen(Qt::black, 2, Qt::SolidLine);

		QPainter painter(this);
		painter.setPen(pen);
		painter.drawLine(QLine(x - crossRadiusPixels, y, x + crossRadiusPixels, y));
		painter.drawLine(QLine(x, y - crossRadiusPixels, x, y + crossRadiusPixels));
	}

	void mousePressEvent(QMouseEvent* event) override
	{
		mouseMoveEvent(event);
	}

	void mouseMoveEvent(QMouseEvent* event) override
	{
		if (event->buttons() & Qt::LeftButton)
		{
			glm::vec2 pos = glm::vec2(float(event->x()) / float(width()), float(event->y()) / float(height())) * 2.0f - 1.0f;
			pos = glm::clamp(pos, glm::vec2(-1.0), glm::vec2(1.0));

			mSetter(pos);
			repaint();
			event->accept();
		}
	}

	void mouseReleaseEvent(QMouseEvent* event) override
	{
		if (mResetOnReleaseValue)
		{
			mSetter(*mResetOnReleaseValue);
			repaint();
			event->accept();
		}
	}

private:
	Vec2Getter mGetter;
	Vec2Setter mSetter;
	boost::optional<glm::vec2> mResetOnReleaseValue;
};

typedef std::function<float()> FloatGetter;
typedef std::function<void(float)> FloatSetter;

class MouseXAxisWidget : public QWidget
{
public:
	MouseXAxisWidget(const FloatGetter& getter, const FloatSetter& setter, QWidget* parent = nullptr) :
		QWidget(parent),
		mGetter(getter),
		mSetter(setter)
	{
		setBackgroundRole(QPalette::Base);
		setAutoFillBackground(true);
	}

	void setResetOnReleaseValue(const float& v)
	{
		mResetOnReleaseValue = v;
	}

private:
	void paintEvent(QPaintEvent* event) override
	{
		static const int crossRadiusPixels = 5;

		float pos = mGetter();
		int x = pos * width();
		QPen pen(Qt::black, 2, Qt::SolidLine);

		QPainter painter(this);
		painter.setPen(pen);
		painter.drawLine(QLine(x, 0, x, height()));
	}

	void mousePressEvent(QMouseEvent* event) override
	{
		mouseMoveEvent(event);
	}

	void mouseMoveEvent(QMouseEvent* event) override
	{
		if (event->buttons() & Qt::LeftButton)
		{
			float x = float(event->x()) / float(width());
			x = glm::clamp(x, 0.0f, 1.0f);

			mSetter(x);
			repaint();
			event->accept();
		}
	}

	void mouseReleaseEvent(QMouseEvent* event) override
	{
		if (mResetOnReleaseValue)
		{
			mSetter(*mResetOnReleaseValue);
			repaint();
			event->accept();
		}
	}

private:
	FloatGetter mGetter;
	FloatSetter mSetter;
	boost::optional<float> mResetOnReleaseValue;
};

class EntityControllerWidget : public QWidget
{
public:
	EntityControllerWidget(QWidget* parent = nullptr) :
		QWidget(parent)
	{
		mLayout = new QGridLayout();
		setLayout(mLayout);

		QTimer *timer = new QTimer(this);
		connect(timer, &QTimer::timeout, this, [this](){
			for (QWidget* widget : mUpdateWidgets)
			{
				widget->repaint();
			}
		});
		timer->start(50);
	}

	//! @param object may be null
	void setEntity(sim::Entity* entity)
	{
		clearLayout(*mLayout);
		mUpdateWidgets.clear();

		if (entity)
		{
			int row = 0;
			addItem("Name", new QLabel(QString::fromStdString(sim::getName(*entity))));

			if (auto component = entity->getFirstComponent<sim::ControlInputsComponent>())
			{
				for (const auto& item : component->controls)
				{
					QString name = QString::fromStdString(item.first);
					sim::ControlInputPtr input = item.second;
					
					if (auto floatControl = dynamic_cast<sim::ControlInputT<float>*>(input.get()))
					{
						auto getter = [floatControl] {return sim::getUnitNormalized(*floatControl); };
						auto setter = [floatControl](float v) { sim::setUnitNormalized<float>(*floatControl, v); };
						auto widget = new MouseXAxisWidget(getter, setter);
						widget->setFixedWidth(mControlSizePixels);
						addItem(name, widget);
					}
					else if (auto vec2Control = dynamic_cast<sim::ControlInputT<glm::vec2>*>(input.get()))
					{
						auto getter = [vec2Control] {return vec2Control->value; };
						auto setter = [vec2Control](const glm::vec2& v) { vec2Control->value = v; };
						auto widget = new MouseXYAxisWidget(getter, setter);
						widget->setFixedSize(QSize(mControlSizePixels, mControlSizePixels));
						widget->setResetOnReleaseValue(glm::vec2(0));
						addItem(name, widget);
					}

				}
			}
		}
		mLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding), mLayout->rowCount(), 0);
	}

private:
	void addItem(const QString& name, QWidget* widget)
	{
		int row = mLayout->rowCount();
		mLayout->addWidget(new QLabel(name), row, 0, Qt::AlignLeft);
		mLayout->addWidget(widget, row, 1, Qt::AlignLeft);
		mUpdateWidgets.push_back(widget);
	}

private:
	QGridLayout* mLayout;
	std::vector<QWidget*> mUpdateWidgets;
	static const int mControlSizePixels = 100;
};

class JoystickInputSystem : public sim::System, public sim::EntityListener
{
public:
	JoystickInputSystem(InputPlatformPtr inputPlatform) :
		mInputPlatform(inputPlatform)
	{
		assert(mInputPlatform);
	}

	~JoystickInputSystem() override
	{
		if (mEntity)
		{
			mEntity->removeListener(this);
		}
	}

	//! @param object may be null
	void setEntity(sim::Entity* entity)
	{
		if (mEntity)
		{
			mEntity->removeListener(this);
		}

		mEntity = entity;

		if (mEntity)
		{
			mEntity->addListener(this);
		}
	}

	void onDestroy(sim::Entity* entity) override
	{
		setEntity(nullptr);
	}

	void updatePreDynamics(const sim::System::StepArgs& args) override
	{
		if (!mEntity)
		{
			return;
		}

		auto devices = mInputPlatform->getInputDevicesOfType(skybolt::InputDeviceTypeJoystick);
		if (!devices.empty())
		{
			auto joystick = devices[0];
			int axisCount = joystick->getAxisCount();
			if (auto component = mEntity->getFirstComponent<sim::ControlInputsComponent>())
			{
				if (axisCount >= 2)
				{
					component->setIfPresent("stick", glm::vec2(joystick->getAxisState(0), joystick->getAxisState(1)) * 2.0f - 1.0f);

					if (axisCount >= 3)
					{
						component->setIfPresent("collective", joystick->getAxisState(2));
						component->setIfPresent("throttle", joystick->getAxisState(2));

						if (axisCount >= 4)
						{
							float value = joystick->getAxisState(3) * 2.0f - 1.0f;
							component->setIfPresent("rudder", value);
							component->setIfPresent("pedal", value);
						}
					}
				}
			}
		}
	}

private:
	InputPlatformPtr mInputPlatform;
	sim::Entity* mEntity = nullptr;
};

class EntityControllerPlugin : public EditorPlugin
{
public:
	EntityControllerPlugin(const EditorPluginConfig& config) :
		mJoystickInputSystem(std::make_shared<JoystickInputSystem>(config.inputPlatform)),
		mEngineRoot(config.engineRoot)
	{
		config.engineRoot->systemRegistry->push_back(mJoystickInputSystem);

		mEntityControllerWidget = new EntityControllerWidget();

		mToolWindow.name = "Entity Controller";
		mToolWindow.widget = mEntityControllerWidget;
	}

	~EntityControllerPlugin()
	{
		VectorUtility::eraseFirst<sim::SystemPtr>(*mEngineRoot->systemRegistry, mJoystickInputSystem);
		delete mToolWindow.widget;
	}

	std::vector<ToolWindow> getToolWindows() override
	{
		return { mToolWindow };
	}

	void explorerSelectionChanged(const TreeItem& item) override
	{
		if (auto entityItem = dynamic_cast<const EntityTreeItem*>(&item))
		{
			sim::Entity* entity = entityItem->data;
			mEntityControllerWidget->setEntity(entity);
			mJoystickInputSystem->setEntity(entity);
		}
		else
		{
			mEntityControllerWidget->setEntity(nullptr);
			mJoystickInputSystem->setEntity(nullptr);
		}
	}

private:
	EngineRoot* mEngineRoot;
	std::shared_ptr<JoystickInputSystem> mJoystickInputSystem;
	EntityControllerWidget* mEntityControllerWidget;
	ToolWindow mToolWindow;
};

namespace plugins {

	std::shared_ptr<EditorPlugin> createEditorPlugin(const EditorPluginConfig& config)
	{
		return std::make_shared<EntityControllerPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEditorPlugin,
		createEditorPlugin
	)
}
