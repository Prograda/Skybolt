/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SkyboltEditorWidgets.h"
#include "Widgets/PositionEditor.h"

#include <SkyboltWidgets/Property/EditorWidgets.h>
#include <SkyboltWidgets/Property/QtProperty.h>
#include <SkyboltWidgets/Property/QtPropertyMetadata.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/PropertyMetadata.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltSim/Spatial/Position.h>

using namespace skybolt;

class LatLonPropertyEditor : public DoubleVectorEditor
{
public:
	LatLonPropertyEditor(QtProperty* property, QWidget* parent = nullptr) :
		mProperty(property),
		DoubleVectorEditor({"Latitude", "Longitude"}, parent)
	{
		setValue(mProperty->value().value<sim::LatLon>());

		connect(property, &QtProperty::valueChanged, this, [=]() {
			setValue(mProperty->value().value<sim::LatLon>());
		});
	}

protected:
	void setValue(const sim::LatLon& value)
	{
		for (int i = 0; i < 2; ++i)
		{
			DoubleVectorEditor::setValue(i, value[i]);
		}
	}

	void componentEdited(int index, double value) override
	{
		sim::LatLon v = mProperty->value().value<sim::LatLon>();
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
	widget->setPosition(sim::GeocentricPosition(toSimVector3(property->value().value<QVector3D>())));

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setPosition(sim::GeocentricPosition(toSimVector3(property->value().value<QVector3D>())));
		widget->blockSignals(false);
	});

	QObject::connect(widget, &PositionEditor::valueChanged, property, [=] (const sim::Position& position) {
		property->setValue(toQVector3D(sim::toGeocentric(position).position));
	});
	return widget;
}

static QWidget* createSkyboltVector3DEditor(QtProperty* property, QWidget* parent)
{
	if (auto value = property->property(QtPropertyMetadataKeys::representation); value.isValid())
	{
		if (value.toString() == sim::PropertyRepresentations::worldPosition)
		{
			return createWorldPositionEditor(property, parent);
		}
	}
	return new QVector3PropertyEditor(property, { "x", "y", "z" }, parent);
}

static QWidget* createLatLonEditor(QtProperty* property, QWidget* parent)
{
	return new LatLonPropertyEditor(property, parent);
}

std::unique_ptr<PropertyEditorWidgetFactoryMap> createSkyboltEditorWidgetFactoryMap(const DefaultEditorWidgetFactoryMapConfig& config)
{
	auto factoryMap = createDefaultEditorWidgetFactoryMap(config);

	(*factoryMap)[QMetaType::Type::QVector3D] = &createSkyboltVector3DEditor;
	(*factoryMap)[qMetaTypeId<sim::LatLon>()] = &createLatLonEditor;

	return factoryMap;
}