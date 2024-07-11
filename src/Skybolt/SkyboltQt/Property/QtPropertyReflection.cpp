/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtPropertyReflection.h"
#include "QtMetaTypes.h"
#include "Property/QtPropertyMetadata.h"

#include "SkyboltQt/QtUtil/QtTypeConversions.h"
#include <SkyboltCommon/Units.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltEngine/Components/VisObjectsComponent.h>
#include <SkyboltReflection/Reflection.h>
#include <SkyboltSim/PropertyMetadata.h>
#include <SkyboltSim/Spatial/LatLon.h>

#include <QVector3D>

using namespace skybolt;

template <typename T>
std::optional<T> simToDisplayUnitsMultiplier(const refl::Property& property, T v)
{
	if (auto value = refl::getOptionalValue<Units>(property.getMetadata(sim::PropertyMetadataNames::units)); value)
	{
		if (*value == Units::Radians)
		{
			return (T)skybolt::math::radToDegD();
		}
	}
	return std::nullopt;
}

template <typename T>
T simUnitToDisplay(const refl::Property& property, T v)
{
	if (auto m = simToDisplayUnitsMultiplier(property, v); m)
	{
		return v * m.value();
	}
	return v;
}

template <typename T>
T displayUnitToSim(const refl::Property& property, T v)
{
	if (auto m = simToDisplayUnitsMultiplier(property, v); m)
	{
		return v / m.value();
	}
	return v;
}

template <typename SimValueT>
QVariant simValueToQt(const refl::Property& property, const SimValueT& value)
{
	return value;
}

template <typename SimValueT>
SimValueT qtValueToSim(const refl::Property& property, const QVariant& value)
{
	return value.value<SimValueT>();
}

template <>
QVariant simValueToQt(const refl::Property& property, const std::string& value)
{
	return QString::fromStdString(value);
}

template <>
std::string qtValueToSim(const refl::Property& property, const QVariant& value)
{
	return value.toString().toStdString();
}

template <>
QVariant simValueToQt(const refl::Property& property, const float& value)
{
	return simUnitToDisplay(property, value);
}

template <>
float qtValueToSim(const refl::Property& property, const QVariant& value)
{
	return displayUnitToSim(property, value.toFloat());
}

template <>
QVariant simValueToQt(const refl::Property& property, const double& value)
{
	return simUnitToDisplay(property, value);
}

template <>
double qtValueToSim(const refl::Property& property, const QVariant& value)
{
	return displayUnitToSim(property, value.toDouble());
}

template <>
QVariant simValueToQt(const refl::Property& property, const sim::Vector3& value)
{
	return toQVector3D(value);
}

template <>
sim::Vector3 qtValueToSim(const refl::Property& property, const QVariant& value)
{
	return toVector3(value.value<QVector3D>());
}

template <>
QVariant simValueToQt(const refl::Property& property, const sim::Quaternion& value)
{
	glm::dvec3 euler = skybolt::math::eulerFromQuat(value) * skybolt::math::radToDegD();
	return QVector3D(euler.x, euler.y, euler.z);
}

template <>
sim::Quaternion qtValueToSim(const refl::Property& property, const QVariant& value)
{
	sim::Vector3 euler = toVector3(value.value<QVector3D>());
	return skybolt::math::quatFromEuler(euler * skybolt::math::degToRadD());
}

template <>
QVariant simValueToQt(const refl::Property& property, const sim::LatLon& value)
{
	return QVariant::fromValue(sim::LatLon(value.lat * skybolt::math::radToDegD(), value.lon * skybolt::math::radToDegD()));
}

template <>
sim::LatLon qtValueToSim(const refl::Property& property, const QVariant& value)
{
	sim::LatLon simValue = value.value<sim::LatLon>();
	simValue.lat *= skybolt::math::degToRadD();
	simValue.lon *= skybolt::math::degToRadD();
	return simValue;
}

static void addMetadata(QtProperty& qtProperty, const refl::Property& reflProperty)
{
	// TODO: Can we translate these types automatically?
	if (auto value = reflProperty.getMetadata(sim::PropertyMetadataNames::attributeType); value.has_value())
	{
		qtProperty.setProperty(QtPropertyMetadataNames::attributeType, int(std::any_cast<sim::AttributeType>(value)));
	}
	if (auto value = reflProperty.getMetadata(sim::PropertyMetadataNames::multiLine); value.has_value())
	{
		qtProperty.setProperty(QtPropertyMetadataNames::multiLine, std::any_cast<bool>(value));
	}
	if (auto value = reflProperty.getMetadata(sim::PropertyMetadataNames::optionNames); value.has_value())
	{
		qtProperty.setProperty(QtPropertyMetadataNames::optionNames, toQStringList(std::any_cast<std::vector<std::string>>(value)));
	}
	if (auto value = reflProperty.getMetadata(sim::PropertyMetadataNames::allowCustomOptions); value.has_value())
	{
		qtProperty.setProperty(QtPropertyMetadataNames::allowCustomOptions, std::any_cast<bool>(value));
	}
}

using PropertyFactory = std::function<QtPropertyUpdaterApplier(refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property)>;

template <typename SimValueT, typename QtValueT>
PropertyFactory createPropertyFactory(const QtValueT& defaultValue)
{
	return [defaultValue] (refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property) {
		QtPropertyUpdaterApplier r;
		
		QString name = QString::fromStdString(property->getName());
		r.property = createQtProperty(name, defaultValue);
		r.property->enabled = !property->isReadOnly();
		addMetadata(*r.property, *property);
		
		r.updater = [instanceGetter, property] (QtProperty& qtProperty) {
			if (const auto& instance = instanceGetter(); instance)
			{
				refl::Instance valueInstance = property->getValue(*instance);
				SimValueT value = *valueInstance.getObject<SimValueT>();
				qtProperty.setValue(simValueToQt(*property, value));
			}
		};
		r.updater(*r.property);

		if (!property->isReadOnly())
		{
			r.applier = [typeRegistry = &typeRegistry, instanceGetter, property] (const QtProperty& qtProperty) {
				if (auto instance = instanceGetter(); instance)
				{
					auto value = qtValueToSim<SimValueT>(*property, static_cast<const QtProperty&>(qtProperty).value);
					property->setValue(*instance, refl::createOwningInstance(typeRegistry, value));
				}
			};
		}

		return r;
	};
}

template <typename SimValueT, typename QtValueT>
PropertyFactory createOptionalPropertyFactory(const QtValueT& defaultValue)
{
	return [defaultValue, basePropertyFactory = createPropertyFactory<SimValueT, QtValueT>(defaultValue)] (refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property) {
		QString name = QString::fromStdString(property->getName());
		QtPropertyPtr baseProperty = createQtProperty(name, defaultValue);
		addMetadata(*baseProperty, *property);

		OptionalProperty optionalProperty;
		optionalProperty.property = baseProperty;

		QtPropertyUpdaterApplier r;
		r.property = createQtProperty(name, QVariant::fromValue(optionalProperty));
		r.property->enabled = !property->isReadOnly();
		QObject::connect(baseProperty.get(), &QtProperty::valueChanged, r.property.get(), &QtProperty::valueChanged);

		r.updater = [instanceGetter, property, defaultValue] (QtProperty& qtProperty) {
			if (const auto& instance = instanceGetter(); instance)
			{
				refl::Instance valueInstance = property->getValue(*instance);
				std::optional<SimValueT> value = *valueInstance.getObject<std::optional<SimValueT>>();

				OptionalProperty optionalProperty = qtProperty.value.value<OptionalProperty>();
				optionalProperty.property->setValue(simValueToQt(*property, value.value_or(defaultValue)));
				optionalProperty.present = value.has_value();
				qtProperty.value.setValue(optionalProperty);
			}
		};
		r.updater(*r.property);

		if (!property->isReadOnly())
		{
			r.applier = [typeRegistry = &typeRegistry, instanceGetter, property] (const QtProperty& qtProperty) {
				if (auto instance = instanceGetter(); instance)
				{
					std::optional<SimValueT> value;

					OptionalProperty optionalProperty = qtProperty.value.value<OptionalProperty>();
					if (optionalProperty.present)
					{
						value = qtValueToSim<SimValueT>(*property, optionalProperty.property->value);
					}
					property->setValue(*instance, refl::createOwningInstance(typeRegistry, value));
				}
			};
		}

		return r;
	};
}

std::optional<QtPropertyUpdaterApplier> reflPropertyToQt(refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property)
{
	const auto& type = property->getType();

	std::map<refl::TypePtr, PropertyFactory> typePropertyFactories = {
		{ typeRegistry.getOrCreateType<std::string>(), createPropertyFactory<std::string>("") },
		{ typeRegistry.getOrCreateType<bool>(), createPropertyFactory<bool>(false) },
		{ typeRegistry.getOrCreateType<int>(), createPropertyFactory<int>(0) },
		{ typeRegistry.getOrCreateType<float>(), createPropertyFactory<float>(0.f) },
		{ typeRegistry.getOrCreateType<double>(), createPropertyFactory<double>(0.0) },
		{ typeRegistry.getOrCreateType<std::optional<std::string>>(), createPropertyFactory<std::string>("") },
		{ typeRegistry.getOrCreateType<std::optional<bool>>(), createOptionalPropertyFactory<bool>(false) },
		{ typeRegistry.getOrCreateType<std::optional<int>>(), createOptionalPropertyFactory<int>(0) },
		{ typeRegistry.getOrCreateType<std::optional<float>>(), createOptionalPropertyFactory<float>(0.f) },
		{ typeRegistry.getOrCreateType<std::optional<double>>(), createOptionalPropertyFactory<double>(0.0) },
		{ typeRegistry.getOrCreateType<sim::Vector3>(), createPropertyFactory<sim::Vector3>(QVector3D(0,0,0)) },
		{ typeRegistry.getOrCreateType<sim::Quaternion>(), createPropertyFactory<sim::Quaternion>(QVector3D(0,0,0)) },
		{ typeRegistry.getOrCreateType<sim::LatLon>(), createPropertyFactory<sim::LatLon>(QVariant::fromValue(sim::LatLon(0,0))) }
	};

	if (const auto& i = typePropertyFactories.find(type); i != typePropertyFactories.end())
	{
		return i->second(typeRegistry, instanceGetter, property); 
	}
	return std::nullopt;
}

void addRttrPropertiesToModel(refl::TypeRegistry& typeRegistry, PropertiesModel& model, const std::vector<refl::PropertyPtr>& properties, const ReflInstanceGetter& instanceGetter)
{
	for (const refl::PropertyPtr& property : properties)
	{
		std::optional<QtPropertyUpdaterApplier> qtProperty = reflPropertyToQt(typeRegistry, instanceGetter, property);
		if (qtProperty)
		{
			model.addProperty(qtProperty->property, qtProperty->updater, qtProperty->applier);
		}
	}
}