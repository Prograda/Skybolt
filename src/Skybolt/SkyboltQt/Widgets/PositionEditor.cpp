/* Copyright Matthew Reid
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

class PositionEditorImpl : public QWidget
{
	Q_OBJECT
public:
	PositionEditorImpl(QWidget* parent = nullptr) : QWidget(parent) {}

	virtual void setPosition(const sim::Position& position) = 0;
	virtual sim::PositionPtr getPosition() const = 0;

	Q_SIGNAL void valueChanged(const sim::Position& position);

protected:
	QLineEdit* addDoubleEditor(QGridLayout& layout, const QString& name)
	{
		int row = layout.rowCount();
		layout.addWidget(new QLabel(name, this), row, 0);
	
		QLineEdit* editor = new QLineEdit(this);
		layout.addWidget(editor, row, 1);

		QDoubleValidator* validator = new QDoubleValidator();
		validator->setNotation(QDoubleValidator::StandardNotation);
		validator->setDecimals(decimalCount);
		editor->setValidator(validator);

		connect(editor, &QLineEdit::editingFinished, this, [this] {
			sim::PositionPtr position = getPosition();
			emit valueChanged(*position);
		});

		return editor;
	}
};

class GeocentricPositionEditor : public PositionEditorImpl
{
public:
	GeocentricPositionEditor(QWidget* parent = nullptr) :
		PositionEditorImpl(parent)
	{
		QGridLayout* layout = new QGridLayout(this);
		layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
		layout->setMargin(0);
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "x");
		mValues[1] = addDoubleEditor(*layout, "y");
		mValues[2] = addDoubleEditor(*layout, "z");
	}

	void setPosition(const sim::Position& position) override
	{
		sim::GeocentricPosition pos = sim::toGeocentric(position);
		for (int i = 0; i < 3; ++i)
		{
			mValues[i]->setText(QString::number(pos.position[i], 'g', decimalCount));
		}
		emit valueChanged(position);
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

class LatLonAltPositionEditor : public PositionEditorImpl
{
public:
	LatLonAltPositionEditor(QWidget* parent = nullptr) :
		PositionEditorImpl(parent)
	{
		QGridLayout* layout = new QGridLayout(this);
		layout->setMargin(0);
		layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "Latitude");
		mValues[1] = addDoubleEditor(*layout, "Longitude");
		mValues[2] = addDoubleEditor(*layout, "Altitude");
	}

	void setPosition(const sim::Position& position) override
	{
		sim::LatLonAltPosition pos = sim::toLatLonAlt(position);
		mValues[0]->setText(QString::number(pos.position.lat * math::radToDegD()));
		mValues[1]->setText(QString::number(pos.position.lon * math::radToDegD()));
		mValues[2]->setText(QString::number(pos.position.alt));

		emit valueChanged(position);
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
	auto layout = new QVBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);

	QComboBox* positionTypeSelector = new QComboBox(this);
	positionTypeSelector->addItems({ "Geographic", "Geocentric" });
	layout->addWidget(positionTypeSelector);

	mStackedWidget = new QStackedWidget(this);
	mStackedWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	addEditor(new LatLonAltPositionEditor());
	addEditor(new GeocentricPositionEditor(this));
	layout->addWidget(mStackedWidget);

	connect(positionTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), mStackedWidget, [=](int index) {
		auto position = getCurrentEditor()->getPosition();
		mStackedWidget->setCurrentIndex(index);
		getCurrentEditor()->setPosition(*position);
	});
}

void PositionEditor::addEditor(PositionEditorImpl* editor)
{
	mStackedWidget->addWidget(editor);
	editor->setMaximumHeight(50);
	editor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	connect(editor, &PositionEditorImpl::valueChanged, this, [this] (const sim::Position& position) {
		emit valueChanged(position);
	});
}

void PositionEditor::setPosition(const sim::Position& position)
{
	getCurrentEditor()->setPosition(position);
}

sim::PositionPtr PositionEditor::getPosition()
{
	return getCurrentEditor()->getPosition();
}

class PositionEditorImpl* PositionEditor::getCurrentEditor() const
{
	return static_cast<PositionEditorImpl*>(mStackedWidget->currentWidget());
}

#include "PositionEditor.moc"