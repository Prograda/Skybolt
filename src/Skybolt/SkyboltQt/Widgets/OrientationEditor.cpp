/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OrientationEditor.h"
#include <SkyboltSim/Spatial/Orientation.h>
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

class Orientable
{
public:
	virtual void setOrientation(const sim::OrientationPtr& orientation, const sim::LatLon& latLon) = 0;
	virtual sim::OrientationPtr getOrientation() const = 0;
};

class GeocentricOrientationEditor : public QWidget, public Orientable
{
public:
	GeocentricOrientationEditor()
	{
		QGridLayout* layout = new QGridLayout;
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "X");
		mValues[1] = addDoubleEditor(*layout, "Y");
		mValues[2] = addDoubleEditor(*layout, "Z");
		mValues[2] = addDoubleEditor(*layout, "W");
	}

	void setOrientation(const sim::OrientationPtr& orientation, const sim::LatLon& latLon) override
	{
		sim::GeocentricOrientation ori = sim::toGeocentric(*orientation, latLon);
		for (int i = 0; i < 4; ++i)
		{
			mValues[i]->setText(QString::number(ori.orientation[i], 'g', decimalCount));
		}
	}

	sim::OrientationPtr getOrientation() const override
	{
		glm::dquat ori;
		for (int i = 0; i < 4; ++i)
		{
			ori[i] = mValues[i]->text().toDouble();
		}
		return std::make_shared<sim::GeocentricOrientation>(ori);
	}

private:
	QLineEdit* mValues[4];
};

class LtpNedOrientationEditor : public QWidget, public Orientable
{
public:
	LtpNedOrientationEditor()
	{
		QGridLayout* layout = new QGridLayout;
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "Roll");
		mValues[1] = addDoubleEditor(*layout, "Pitch");
		mValues[2] = addDoubleEditor(*layout, "Yaw");
	}

	void setOrientation(const sim::OrientationPtr& orientation, const sim::LatLon& latLon) override
	{
		sim::LtpNedOrientation ori = sim::toLtpNed(*orientation, latLon);
		glm::dvec3 rpy = math::eulerFromQuat(ori.orientation) * math::radToDegD();
		for (int i = 0; i < 3; ++i)
		{
			mValues[i]->setText(QString::number(rpy[i], 'g', decimalCount));
		}
	}

	sim::OrientationPtr getOrientation() const override
	{
		glm::dvec3 rpy;
		for (int i = 0; i < 3; ++i)
		{
			rpy[i] = mValues[i]->text().toDouble() * math::degToRadD();
		}
		return std::make_shared<sim::LtpNedOrientation>(math::quatFromEuler(rpy));
	}

private:
	QLineEdit* mValues[3];
};
OrientationEditor::OrientationEditor(QWidget* parent) :
	QWidget(parent)
{
	setLayout(new QVBoxLayout);

	QComboBox* positionTypeSelector = new QComboBox;
	positionTypeSelector->addItems({ "LTP Euler", "Geocentric" });
	layout()->addWidget(positionTypeSelector);

	mStackedWidget = new QStackedWidget;
	mStackedWidget->addWidget(new LtpNedOrientationEditor);
	mStackedWidget->addWidget(new GeocentricOrientationEditor);
	layout()->addWidget(mStackedWidget);

	connect(positionTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), mStackedWidget, [=](int index) {
		auto orientation = getCurrentEditor()->getOrientation();
		mStackedWidget->setCurrentIndex(index);
		getCurrentEditor()->setOrientation(orientation, mLatLon);
	});
}

void OrientationEditor::setOrientation(const sim::OrientationPtr& orientation, const sim::LatLon& latLon)
{
	mLatLon = latLon;
	getCurrentEditor()->setOrientation(orientation, latLon);
}

sim::OrientationPtr OrientationEditor::getOrientation()
{
	return getCurrentEditor()->getOrientation();
}

class Orientable* OrientationEditor::getCurrentEditor() const
{
	return dynamic_cast<Orientable*>(mStackedWidget->currentWidget());
}
