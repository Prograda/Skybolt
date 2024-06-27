/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DefaultEditorWidgets.h"
#include "PropertyEditor.h"
#include "QtPropertyMetadata.h"
#include "QtMetaTypes.h"
#include "SkyboltQt/Widgets/PositionEditor.h"

#include <SkyboltReflection/Reflection.h>
#include <SkyboltSim/PropertyMetadata.h>
#include <SkyboltSim/Spatial/Position.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QItemEditorFactory>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVector3D>

using namespace skybolt;

static QLineEdit* createDoubleLineEdit(QWidget* parent = nullptr)
{
	QLineEdit* editor = new QLineEdit(parent);

	QDoubleValidator* validator = new QDoubleValidator();
	validator->setNotation(QDoubleValidator::StandardNotation);
	editor->setValidator(validator);

	return editor;
}

static QLineEdit* addDoubleEditor(QGridLayout& layout, const QString& name)
{
	int row = layout.rowCount();
	layout.addWidget(new QLabel(name), row, 0);

	QLineEdit* editor = createDoubleLineEdit();
	layout.addWidget(editor, row, 1);

	return editor;
}

class VectorEditor : public QWidget
{
public:
	VectorEditor(const QStringList& componentLabels, QWidget* parent = nullptr) :
		QWidget(parent)
	{
		QGridLayout* layout = new QGridLayout;
		layout->setMargin(0);
		setLayout(layout);
		
		for (const QString& label : componentLabels)
		{
			int index = (int)mEditors.size();
			mEditors.push_back(addDoubleEditor(*layout, label));
			connect(mEditors.back(), &QLineEdit::editingFinished, this, [this, index]() {
				componentEdited(index, mEditors[index]->text().toDouble());
			});
		}
	}

protected:
	void setValue(int index, double value)
	{
		mEditors[index]->blockSignals(true);
		mEditors[index]->setText(QString::number(value));
		mEditors[index]->blockSignals(false);
	}

	virtual void componentEdited(int index, double value) = 0;

private:
	std::vector<QLineEdit*> mEditors;
};

class QVector3PropertyEditor : public VectorEditor
{
public:
	QVector3PropertyEditor(QtProperty* property, const QStringList& componentLabels, QWidget* parent = nullptr) :
		mProperty(property),
		VectorEditor(componentLabels, parent)
	{
		setValue(mProperty->value.value<QVector3D>());

		connect(property, &QtProperty::valueChanged, this, [=]() {
			setValue(mProperty->value.value<QVector3D>());
		});
	}

protected:
	void setValue(const QVector3D& value)
	{
		for (int i = 0; i < 3; ++i)
		{
			VectorEditor::setValue(i, value[i]);
		}
	}

	void componentEdited(int index, double value) override
	{
		QVector3D vec = mProperty->value.value<QVector3D>();
		vec[index] = value;
		mProperty->setValue(vec);
	}

private:
	QtProperty* mProperty;
};

class LatLonPropertyEditor : public VectorEditor
{
public:
	LatLonPropertyEditor(QtProperty* property, QWidget* parent = nullptr) :
		mProperty(property),
		VectorEditor({"Latitude", "Longitude"}, parent)
	{
		setValue(mProperty->value.value<sim::LatLon>());

		connect(property, &QtProperty::valueChanged, this, [=]() {
			setValue(mProperty->value.value<sim::LatLon>());
		});
	}

protected:
	void setValue(const sim::LatLon& value)
	{
		for (int i = 0; i < 2; ++i)
		{
			VectorEditor::setValue(i, value[i]);
		}
	}

	void componentEdited(int index, double value) override
	{
		sim::LatLon v = mProperty->value.value<sim::LatLon>();
		v[index] = value;
		mProperty->setValue(QVariant::fromValue(v));
	}

private:
	QtProperty* mProperty;
};

static sim::Vector3 toSimVector3(const QVector3D& v)
{
	return sim::Vector3(v.x(), v.y(), v.z());
}

static QVector3D toQVector3D(const sim::Vector3& v)
{
	return QVector3D(v.x, v.y, v.z);
}

static PositionEditor* createWorldPositionEditor(QtProperty* property, QWidget* parent)
{
	auto widget = new PositionEditor(parent);
	widget->setPosition(sim::GeocentricPosition(toSimVector3(property->value.value<QVector3D>())));

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setPosition(sim::GeocentricPosition(toSimVector3(property->value.value<QVector3D>())));
		widget->blockSignals(false);
	});

	QObject::connect(widget, &PositionEditor::valueChanged, property, [=] (const sim::Position& position) {
		property->setValue(toQVector3D(sim::toGeocentric(position).position));
	});
	return widget;
}

static QWidget* createComboStringEditor(QtProperty* property, QWidget* parent)
{
	QStringList optionNames = property->property(QtPropertyMetadataNames::optionNames).toStringList();
	auto widget = new QComboBox(parent);
	widget->addItems(optionNames);
	widget->setCurrentText(property->value.toString());
	widget->setEditable(property->property(QtPropertyMetadataNames::allowCustomOptions).toBool());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setCurrentText(property->value.toString());
		widget->blockSignals(false);
	});

	QObject::connect(widget, &QComboBox::currentTextChanged, property, [=](const QString& newValue) {
		property->setValue(newValue);
	});

	return widget;
}

static QWidget* createSingleLineStringEditor(QtProperty* property, QWidget* parent)
{
	QLineEdit* widget = new QLineEdit(parent);
	widget->setText(property->value.toString());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setText(property->value.toString());
		widget->blockSignals(false);
	});

	QObject::connect(widget, &QLineEdit::editingFinished, property, [=]() {
		property->setValue(widget->text());
	});

	return widget;
}

static QWidget* createMultiLineStringEditor(QtProperty* property, QWidget* parent)
{
	QTextEdit* widget = new QTextEdit(parent);
	widget->setText(property->value.toString());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setText(property->value.toString());
		widget->blockSignals(false);
	});

	QObject::connect(widget, &QTextEdit::textChanged, property, [=]() {
		property->setValue(widget->toPlainText());
	});

	return widget;
}


static QWidget* createStringEditor(QtProperty* property, QWidget* parent)
{
	if (auto value = property->property(QtPropertyMetadataNames::optionNames); value.isValid())
	{
		return createComboStringEditor(property, parent);
	}

	if (auto value = property->property(QtPropertyMetadataNames::multiLine); value.isValid() && value.toBool())
	{
		return createMultiLineStringEditor(property, parent);
	}
	else
	{
		return createSingleLineStringEditor(property, parent);
	}
}

static QWidget* createIntEditor(QtProperty* property, QWidget* parent)
{
	QSpinBox* widget = new QSpinBox(parent);
	widget->setMaximum(999999);
	widget->setValue(property->value.toInt());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setValue(property->value.toInt());
		widget->blockSignals(false);
	});

	QObject::connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), property, [=](int newValue) {
		property->setValue(newValue);
	});

	return widget;
}

static QWidget* createEnumEditor(QtProperty* property, QWidget* parent)
{
	QStringList optionNames = property->property(QtPropertyMetadataNames::optionNames).toStringList();
	auto widget = new QComboBox(parent);
	widget->addItems(optionNames);
	widget->setCurrentIndex(property->value.toInt());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setCurrentIndex(property->value.toInt());
		widget->blockSignals(false);
	});

	QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), property, [=](int newValue) {
		property->setValue(newValue);
	});

	return widget;
}

static bool shouldUseEnumEditor(const QtProperty& property)
{
	if (property.property(QtPropertyMetadataNames::optionNames).isValid())
	{
		if (auto allowCustomOptions = property.property(QtPropertyMetadataNames::allowCustomOptions); allowCustomOptions.isValid())
		{
			if (allowCustomOptions.toBool())
			{
				// Can't use enum editor if custom options are allowed.
				return false;
			}
		}
		return true;
	}
	return false;
}

static QWidget* createIntOrEnumEditor(QtProperty* property, QWidget* parent)
{
	if (shouldUseEnumEditor(*property))
	{
		return createEnumEditor(property, parent);
	}
	else
	{
		return createIntEditor(property, parent);
	}
}

static QWidget* createDoubleEditor(QtProperty* property, QWidget* parent)
{
	QLineEdit* widget = createDoubleLineEdit(parent);

	auto widgetTextSetter = [widget](double value) {
		widget->setText(QString::number(value, 'f', 4));
	};
	widgetTextSetter(property->value.toDouble());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property, widgetTextSetter]() {
		widget->blockSignals(true);
		widgetTextSetter(property->value.toDouble());
		widget->blockSignals(false);
	});

	QObject::connect(widget, &QLineEdit::editingFinished, property, [=]() {
		property->setValue(widget->text().toDouble());
	});

	return widget;
}

static QWidget* createBoolEditor(QtProperty* property, QWidget* parent)
{
	QCheckBox* widget = new QCheckBox(parent);
	widget->setChecked(property->value.toBool());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setChecked(property->value.toBool());
		widget->blockSignals(false);
	});

	QObject::connect(widget, &QCheckBox::stateChanged, property, [=](int state) {
		property->setValue((bool)state);
	});

	return widget;
}

static QWidget* createDateTimeEditor(QtProperty* property, QWidget* parent)
{
	QDateTimeEdit* widget = new QDateTimeEdit(parent);
	widget->setDateTime(property->value.toDateTime());

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setDateTime(property->value.toDateTime());
		widget->blockSignals(false);
	});

	QObject::connect(widget, &QDateTimeEdit::dateTimeChanged, property, [=](const QDateTime& dateTime) {
		property->setValue(dateTime);
	});

	return widget;
}

static QWidget* createVector3DEditor(QtProperty* property, QWidget* parent)
{
	if (auto value = property->property(QtPropertyMetadataNames::attributeType); value.isValid() && value.toInt() == int(sim::AttributeType::PositionInWorld))
	{
		return createWorldPositionEditor(property, parent);
	}
	return new QVector3PropertyEditor(property, { "x", "y", "z" }, parent);
}

static QWidget* createLatLonEditor(QtProperty* property, QWidget* parent)
{
	return new LatLonPropertyEditor(property, parent);
}

static QWidget* createOptionalVariantEditor(const PropertyEditorWidgetFactoryMap& factories, QtProperty* property, QWidget* parent)
{
	auto optionalProperty = property->value.value<OptionalProperty>();
	if (auto i = factories.find(optionalProperty.property->value.userType()); i != factories.end())
	{
		auto widget = new QWidget(parent);
		auto layout = new QVBoxLayout(widget);
		layout->setMargin(0);
		widget->setLayout(layout);

		auto activateCheckbox = new QCheckBox("Valid", widget);
		activateCheckbox->setChecked(optionalProperty.present);
		QWidget* valueEditorWidget = i->second(optionalProperty.property.get(), parent);
		valueEditorWidget->setEnabled(optionalProperty.present);

		QObject::connect(property, &QtProperty::valueChanged, activateCheckbox, [activateCheckbox, valueEditorWidget, property]() {
			bool present = property->value.value<OptionalProperty>().present;
			activateCheckbox->blockSignals(true);
			activateCheckbox->setChecked(present);
			activateCheckbox->blockSignals(false);
			valueEditorWidget->setEnabled(present && property->enabled);
		});

		QObject::connect(activateCheckbox, &QCheckBox::stateChanged, property, [=](bool value) {
			auto optionalProperty = property->value.value<OptionalProperty>();
			optionalProperty.present = value;
			property->setValue(QVariant::fromValue(optionalProperty));
			valueEditorWidget->setEnabled(value && property->enabled);
		});

		layout->addWidget(activateCheckbox);
		layout->addWidget(valueEditorWidget);
		return widget;
	}
	return nullptr;
}


static PropertyEditorWidgetFactoryMap createDefaultEditorWidgetFactoryMap()
{
	static PropertyEditorWidgetFactoryMap m = {
		{ QMetaType::Type::QString, &createStringEditor },
		{ QMetaType::Type::Int, &createIntOrEnumEditor },
		{ QMetaType::Type::Float, &createDoubleEditor },
		{ QMetaType::Type::Double, &createDoubleEditor },
		{ QMetaType::Type::Bool, &createBoolEditor },
		{ QMetaType::Type::QDateTime, &createDateTimeEditor },
		{ QMetaType::Type::QVector3D, &createVector3DEditor },
		{ qMetaTypeId<sim::LatLon>(), &createLatLonEditor }
	};

	m[qMetaTypeId<OptionalProperty>()] = [factories = &m] (QtProperty* property, QWidget* parent) {
		return createOptionalVariantEditor(*factories, property, parent);
	};

	return m;
}

PropertyEditorWidgetFactoryMap getDefaultEditorWidgetFactoryMap()
{
	static PropertyEditorWidgetFactoryMap m = createDefaultEditorWidgetFactoryMap();
	return m;
}