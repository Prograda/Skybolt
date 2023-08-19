/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PropertyEditor.h"
#include "PropertyMetadata.h"
#include "Sprocket/QtUtil/QtLayoutUtil.h"
#include "Sprocket/Widgets/PositionEditor.h"
#include "Sprocket/Widgets/TableEditorWidget.h"
#include "SkyboltSim/Reflection.h"
#include "SkyboltSim/Spatial/Position.h"

#include <QtCore/QDate>
#include <QtCore/QLocale>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QItemEditorFactory>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVector3D>

#include <assert.h>

using namespace skybolt;

static void showTableEditor(TableProperty& property)
{
	TableEditorWidget* tableEditor = new TableEditorWidget(property.fieldNames);
	tableEditor->setRecords(property.records);

	QDialog* dialog = new QDialog(nullptr, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
	dialog->setWindowTitle("Table Editor");
	dialog->setLayout(new QVBoxLayout());
	dialog->layout()->addWidget(tableEditor);
	dialog->setModal(true);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	dialog->layout()->addWidget(buttonBox);

	QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

	if (dialog->exec() == QDialog::Accepted)
	{
		property.records = tableEditor->getRecords();
		emit property.valueChanged();
	}
}

PropertyEditor::PropertyEditor(const PropertyEditorWidgetFactoryMap& factoryMap, QWidget* parent) :
	QWidget(parent),
	mFactoryMap(factoryMap),
	mGridLayout(new QGridLayout)
{
	setLayout(mGridLayout);
}

void PropertyEditor::setModel(const PropertiesModelPtr& model)
{
	if (mModel)
		disconnect(mModel.get(), SIGNAL(modelReset(PropertiesModel*)), this, SLOT(modelReset(PropertiesModel*)));

	mModel = model;

	if (model)
	{
		connect(model.get(), SIGNAL(modelReset(PropertiesModel*)), this, SLOT(modelReset(PropertiesModel*)));
		modelReset(model.get());
	}
	else
	{
		clearLayout(*layout());
	}
}

void PropertyEditor::modelReset(PropertiesModel* model)
{
	assert(mModel.get() == model);

	clearLayout(*mGridLayout);

	int r = 0;
	for (const QtPropertyPtr& property : model->getProperties())
	{
		mGridLayout->addWidget(new QLabel(property->name), r, 0);
		QWidget* widget = createEditor(*property);
		if (widget)
		{
			mGridLayout->addWidget(widget, r, 1);
			mGridLayout->setRowStretch(r, -1);
		}

		++r;
	}
	mGridLayout->setRowStretch(r, 1);
}

static QLineEdit* createDoubleEditor(QWidget* parent = nullptr)
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

	QLineEdit* editor = createDoubleEditor();
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
			mEditors.push_back(addDoubleEditor(*layout, label));

			connect(mEditors.back(), &QLineEdit::textEdited, this, [this](const QString& text) {
				componentEdited((int)mEditors.size() - 1, text.toDouble());
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
	QVector3PropertyEditor(VariantProperty* property, const QStringList& componentLabels, QWidget* parent = nullptr) :
		mProperty(property),
		VectorEditor(componentLabels, parent)
	{
		setValue(mProperty->value.value<QVector3D>());

		connect(property, &VariantProperty::valueChanged, this, [=]() {
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
	VariantProperty* mProperty;
};

static sim::Vector3 toSimVector3(const QVector3D& v)
{
	return sim::Vector3(v.x(), v.y(), v.z());
}

static QVector3D toQVector3D(const sim::Vector3& v)
{
	return QVector3D(v.x, v.y, v.z);
}

static PositionEditor* createWorldPositionEditor(VariantProperty* variantProperty, QWidget* parent)
{
	auto widget = new PositionEditor(parent);
	widget->setPosition(sim::GeocentricPosition(toSimVector3(variantProperty->value.value<QVector3D>())));

	QObject::connect(variantProperty, &QtProperty::valueChanged, widget, [widget, variantProperty]() {
		widget->blockSignals(true);
		widget->setPosition(sim::GeocentricPosition(toSimVector3(variantProperty->value.value<QVector3D>())));
		widget->blockSignals(false);
	});

	QObject::connect(widget, &PositionEditor::valueChanged, variantProperty, [=] (const sim::Position& position) {
		variantProperty->setValue(toQVector3D(sim::toGeocentric(position).position));
	});
	return widget;
}

QWidget* PropertyEditor::createEditorInEnabledState(QtProperty& property)
{
	// See if a custom editor widget is registered for this property type
	auto i = mFactoryMap.find(typeid(property));
	if (i != mFactoryMap.end())
	{
		return i->second(property);
	}

	// Fall back to default variant editor widget
	if (auto variantProperty = dynamic_cast<VariantProperty*>(&property))
	{
		const QVariant& value = variantProperty->value;
		switch (value.type())
		{
			case QVariant::String:
			{
				QLineEdit* widget = new QLineEdit(this);
				widget->setText(value.toString());

				connect(&property, &QtProperty::valueChanged, widget, [widget, variantProperty]() {
					widget->blockSignals(true);
					widget->setText(variantProperty->value.toString());
					widget->blockSignals(false);
				});

				connect(widget, &QLineEdit::editingFinished, &property, [=]() {
					variantProperty->setValue(widget->text());
				});

				return widget;
			}
			case QVariant::Int:
			{
				QSpinBox* widget = new QSpinBox(this);
				widget->setMaximum(999999);
				widget->setValue(value.toInt());

				connect(&property, &QtProperty::valueChanged, widget, [widget, variantProperty]() {
					widget->blockSignals(true);
					widget->setValue(variantProperty->value.toInt());
					widget->blockSignals(false);
				});

				connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), &property, [=](int newValue) {
					variantProperty->setValue(newValue);
				});

				return widget;
			}
			case QMetaType::Type::Float:
			case QVariant::Double:
			{
				QLineEdit* widget = createDoubleEditor(this);

				auto widgetTextSetter = [widget](double value) {
					widget->setText(QString::number(value, 'f', 4));
				};
				widgetTextSetter(variantProperty->value.toDouble());

				connect(&property, &QtProperty::valueChanged, widget, [widget, variantProperty, widgetTextSetter]() {
					widget->blockSignals(true);
					widgetTextSetter(variantProperty->value.toDouble());
					widget->blockSignals(false);
				});

				connect(widget, &QLineEdit::editingFinished, &property, [=]() {
					variantProperty->setValue(widget->text().toDouble());
				});

				return widget;
			}
			case QVariant::Bool:
			{
				QCheckBox* widget = new QCheckBox(this);
				widget->setChecked(value.toBool());

				connect(&property, &QtProperty::valueChanged, widget, [widget, variantProperty]() {
					widget->blockSignals(true);
					widget->setChecked(variantProperty->value.toBool());
					widget->blockSignals(false);
				});

				connect(widget, &QCheckBox::stateChanged, &property, [=](int state) {
					variantProperty->setValue((bool)state);
				});

				return widget;
			}
			case QVariant::DateTime:
			{
				QDateTimeEdit* widget = new QDateTimeEdit(this);
				widget->setDateTime(value.toDateTime());

				connect(&property, &QtProperty::valueChanged, widget, [widget, variantProperty]() {
					widget->blockSignals(true);
					widget->setDateTime(variantProperty->value.toDateTime());
					widget->blockSignals(false);
				});

				connect(widget, &QDateTimeEdit::dateTimeChanged, &property, [=](const QDateTime& dateTime) {
					variantProperty->setValue(dateTime);
				});

				return widget;
			}
			case QVariant::Vector3D:
			{
				if (auto value = property.property(PropertyMetadataNames::attributeType); value.isValid() && value.toInt() == int(sim::AttributeType::PositionInWorld))
				{
					return createWorldPositionEditor(variantProperty, this);
				}
				return new QVector3PropertyEditor(variantProperty, { "x", "y", "z" }, this);
			}
		}
	}
	else if (auto enumProperty = dynamic_cast<EnumProperty*>(&property))
	{
		QComboBox* widget = new QComboBox(this);
		widget->addItems(enumProperty->values);
		widget->setCurrentIndex(enumProperty->value);

		connect(&property, &QtProperty::valueChanged, widget, [widget, enumProperty]() {
			widget->blockSignals(true);
			widget->setCurrentIndex(enumProperty->value);
			widget->blockSignals(false);
		});

		connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
			enumProperty->setValue(index);
		});

		return widget;
	}
	else if (auto tableProperty = dynamic_cast<TableProperty*>(&property))
	{
		QPushButton* button = new QPushButton("Edit...", this);

		connect(button, &QPushButton::clicked, this, [=]() {
			showTableEditor(*tableProperty);
		});

		return button;
	}
	return nullptr;
}

QWidget* PropertyEditor::createEditor(QtProperty& property)
{
	QWidget* widget = createEditorInEnabledState(property);

	if (widget)
	{
		widget->setEnabled(property.enabled);
		connect(&property, &QtProperty::enabledChanged, widget, [=](bool enabled) {
			widget->setEnabled(enabled);
		});
	}
	return widget;
}
