/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "EntityControllerWidget.h"

#include <SkyboltQt/Scenario/EntityObjectType.h>
#include <SkyboltQt/Scenario/ScenarioSelectionModel.h>
#include <SkyboltQt/QtUtil/QtLayoutUtil.h>

#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltCommon/VectorUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include <memory>
#include <optional>

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
	std::optional<glm::vec2> mResetOnReleaseValue;
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
	std::optional<float> mResetOnReleaseValue;
};

EntityControllerWidget::EntityControllerWidget(QWidget* parent) :
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

EntityControllerWidget::~EntityControllerWidget() = default;

//! @param object may be null
void EntityControllerWidget::setEntity(sim::Entity* entity)
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

void EntityControllerWidget::addItem(const QString& name, QWidget* widget)
{
	int row = mLayout->rowCount();
	mLayout->addWidget(new QLabel(name), row, 0, Qt::AlignLeft);
	mLayout->addWidget(widget, row, 1, Qt::AlignLeft);
	mUpdateWidgets.push_back(widget);
}
