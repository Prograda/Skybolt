/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PositionEditor.h"
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>

using namespace skybolt;
using namespace skybolt::math;

constexpr int decimalCount = 9;

static QLineEdit* addDoubleEditor(QGridLayout& layout, const QString& name)
{
	int row = layout.rowCount();
	layout.addWidget(new QLabel(name), row, 0);
	
	QLineEdit* editor = new QLineEdit;
	layout.addWidget(editor, row, 1);

	QDoubleValidator* validator = new QDoubleValidator();
	validator->setNotation(QDoubleValidator::StandardNotation);
	validator->setDecimals(decimalCount);
	editor->setValidator(validator);

	return editor;
}

class Positionable
{
public:
	virtual void setPosition(const sim::PositionPtr& position) = 0;
	virtual sim::PositionPtr getPosition() const = 0;
};

class GeocentricPositionEditor : public QWidget, public Positionable
{
public:
	GeocentricPositionEditor()
	{
		QGridLayout* layout = new QGridLayout;
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "X");
		mValues[1] = addDoubleEditor(*layout, "Y");
		mValues[2] = addDoubleEditor(*layout, "Z");
	}

	void setPosition(const sim::PositionPtr& position) override
	{
		sim::GeocentricPosition pos = sim::toGeocentric(*position);
		for (int i = 0; i < 3; ++i)
		{
			mValues[i]->setText(QString::number(pos.position[i], 'g', decimalCount));
		}
	}

	sim::PositionPtr getPosition() const override
	{
		sim::Vector3 pos;
		for (int i = 0; i < 3; ++i)
		{
			pos[i] = mValues[i]->text().toDouble();
		}
		return std::make_shared<sim::GeocentricPosition>(pos);
	}

private:
	QLineEdit* mValues[3];
};

class LatLonAltPositionEditor : public QWidget, public Positionable
{
public:
	LatLonAltPositionEditor()
	{
		QGridLayout* layout = new QGridLayout;
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "Latitude");
		mValues[1] = addDoubleEditor(*layout, "Longitude");
		mValues[2] = addDoubleEditor(*layout, "Altitude");
	}

	void setPosition(const sim::PositionPtr& position) override
	{
		sim::LatLonAltPosition pos = sim::toLatLonAlt(*position);
		mValues[0]->setText(QString::number(pos.position.lat * math::radToDegD()));
		mValues[1]->setText(QString::number(pos.position.lon * math::radToDegD()));
		mValues[2]->setText(QString::number(pos.position.alt));
	}

	sim::PositionPtr getPosition() const override
	{
		sim::LatLonAlt lla(
			mValues[0]->text().toDouble() * math::degToRadD(),
			mValues[1]->text().toDouble() * math::degToRadD(),
			mValues[2]->text().toDouble());
		return std::make_shared<sim::LatLonAltPosition>(lla);
	}

private:
	QLineEdit* mValues[3];
};

PositionEditor::PositionEditor(QWidget* parent) :
	QWidget(parent)
{
	setLayout(new QVBoxLayout);

	QComboBox* positionTypeSelector = new QComboBox;
	positionTypeSelector->addItems({ "LatLonAlt", "Geocentric" });
	layout()->addWidget(positionTypeSelector);

	mStackedWidget = new QStackedWidget;
	mStackedWidget->addWidget(new LatLonAltPositionEditor);
	mStackedWidget->addWidget(new GeocentricPositionEditor);
	layout()->addWidget(mStackedWidget);

	connect(positionTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), mStackedWidget, [=](int index) {
		auto position = getCurrentEditor()->getPosition();
		mStackedWidget->setCurrentIndex(index);
		getCurrentEditor()->setPosition(position);
	});
}

void PositionEditor::setPosition(const sim::PositionPtr& position)
{
	getCurrentEditor()->setPosition(position);
}

sim::PositionPtr PositionEditor::getPosition()
{
	return getCurrentEditor()->getPosition();
}

class Positionable* PositionEditor::getCurrentEditor() const
{
	return dynamic_cast<Positionable*>(mStackedWidget->currentWidget());
}
