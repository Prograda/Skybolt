/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "QtPropertyReflection.h"
#include "PropertyMetadata.h"
#include "SprocketMetaTypes.h"
#include "Sprocket/QtTypeConversions.h"
#include <SkyboltCommon/Units.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltEngine/VisObjectsComponent.h>
#include <SkyboltSim/Reflection.h>
#include <SkyboltSim/Spatial/LatLon.h>

#include <QVector3D>

using namespace skybolt;

template <typename T>
std::optional<T> simToDisplayUnitsMultiplier(const rttr::property& property, T v)
{
	auto variant = property.get_metadata(sim::PropertyMetadataType::Units);
	if (variant.is_type<Units>())
	{
		if (variant.get_value<Units>() == Units::Radians)
		{
			return (T)skybolt::math::radToDegD();
		}
	}
	return std::nullopt;
}

template <typename T>
T simUnitToDisplay(const rttr::property& property, T v)
{
	if (auto m = simToDisplayUnitsMultiplier(property, v); m)
	{
		return v * m.value();
	}
	return v;
}

template <typename T>
T displayUnitToSim(const rttr::property& property, T v)
{
	if (auto m = simToDisplayUnitsMultiplier(property, v); m)
	{
		return v / m.value();
	}
	return v;
}

template <typename SimValueT>
QVariant simValueToQt(const rttr::property& property, const SimValueT& value)
{
	return value;
}

template <typename SimValueT>
SimValueT qtValueToSim(const rttr::property& property, const QVariant& value)
{
	return value.value<SimValueT>();
}

template <>
QVariant simValueToQt(const rttr::property& property, const std::string& value)
{
	return QString::fromStdString(value);
}

template <>
std::string qtValueToSim(const rttr::property& property, const QVariant& value)
{
	return value.toString().toStdString();
}

template <>
QVariant simValueToQt(const rttr::property& property, const float& value)
{
	return simUnitToDisplay(property, value);
}

template <>
float qtValueToSim(const rttr::property& property, const QVariant& value)
{
	return displayUnitToSim(property, value.toFloat());
}

template <>
QVariant simValueToQt(const rttr::property& property, const double& value)
{
	return simUnitToDisplay(property, value);
}

template <>
double qtValueToSim(const rttr::property& property, const QVariant& value)
{
	return displayUnitToSim(property, value.toDouble());
}

template <>
QVariant simValueToQt(const rttr::property& property, const sim::Vector3& value)
{
	return toQVector3D(value);
}

template <>
sim::Vector3 qtValueToSim(const rttr::property& property, const QVariant& value)
{
	return toVector3(value.value<QVector3D>());
}

template <>
QVariant simValueToQt(const rttr::property& property, const sim::Quaternion& value)
{
	glm::dvec3 euler = skybolt::math::eulerFromQuat(value) * skybolt::math::radToDegD();
	return QVector3D(euler.x, euler.y, euler.z);
}

template <>
sim::Quaternion qtValueToSim(const rttr::property& property, const QVariant& value)
{
	sim::Vector3 euler = toVector3(value.value<QVector3D>());
	return skybolt::math::quatFromEuler(euler * skybolt::math::radToDegD());
}

template <>
QVariant simValueToQt(const rttr::property& property, const sim::LatLon& value)
{
	return QVariant::fromValue(sim::LatLon(value.lat * skybolt::math::radToDegD(), value.lon * skybolt::math::radToDegD()));
}

template <>
sim::LatLon qtValueToSim(const rttr::property& property, const QVariant& value)
{
	sim::LatLon simValue = value.value<sim::LatLon>();
	simValue.lat *= skybolt::math::degToRadD();
	simValue.lon *= skybolt::math::degToRadD();
	return simValue;
}

static void addMetadata(QtProperty& qtProperty, const rttr::property& rttrProperty)
{
	// TODO: copy metadata from rttr to QT property automatically, without needing to explicitly all types here?
	if (auto variant = rttrProperty.get_metadata(sim::PropertyMetadataType::AttributeType); variant.is_valid())
	{
		qtProperty.setProperty(PropertyMetadataNames::attributeType, variant.get_value<int>());
	}
	if (auto variant = rttrProperty.get_metadata(sim::PropertyMetadataType::MultiLine); variant.is_valid())
	{
		qtProperty.setProperty(PropertyMetadataNames::multiLine, variant.get_value<bool>());
	}
}

using PropertyFactory = std::function<QtPropertyUpdaterApplier(const RttrInstanceGetter& instanceGetter, const rttr::property& property)>;

template <typename SimValueT, typename QtValueT>
PropertyFactory createPropertyFactory(const QtValueT& defaultValue)
{
	return [defaultValue] (const RttrInstanceGetter& instanceGetter, const rttr::property& property) {
		QtPropertyUpdaterApplier r;
		
		QString name = QString::fromStdString(property.get_name().to_string());
		r.property = createQtProperty(name, defaultValue);
		r.property->enabled = !property.is_readonly();
		addMetadata(*r.property, property);
		
		r.updater = [instanceGetter, property] (QtProperty& qtProperty) {
			rttr::instance instance = instanceGetter();
			if (instance.is_valid())
			{
				auto value = property.get_value(instance).get_value<SimValueT>();
				qtProperty.setValue(simValueToQt(property, value));
			}
		};
		r.updater(*r.property);

		if (!property.is_readonly())
		{
			r.applier = [instanceGetter, property] (const QtProperty& qtProperty) {
				rttr::instance instance = instanceGetter();
				if (instance.is_valid())
				{
					auto value = qtValueToSim<SimValueT>(property, static_cast<const QtProperty&>(qtProperty).value);
					property.set_value(instance, value);
				}
			};
		}

		return r;
	};
}

template <typename SimValueT, typename QtValueT>
PropertyFactory createOptionalPropertyFactory(const QtValueT& defaultValue)
{
	return [defaultValue, basePropertyFactory = createPropertyFactory<SimValueT, QtValueT>(defaultValue)] (const RttrInstanceGetter& instanceGetter, const rttr::property& property) {
		QString name = QString::fromStdString(property.get_name().to_string());
		QtPropertyPtr baseProperty = createQtProperty(name, defaultValue);
		addMetadata(*baseProperty, property);

		OptionalProperty optionalProperty;
		optionalProperty.property = baseProperty;

		QtPropertyUpdaterApplier r;
		r.property = createQtProperty(name, QVariant::fromValue(optionalProperty));
		r.property->enabled = !property.is_readonly();
		QObject::connect(baseProperty.get(), &QtProperty::valueChanged, r.property.get(), &QtProperty::valueChanged);

		r.updater = [instanceGetter, property, defaultValue] (QtProperty& qtProperty) {
			rttr::instance instance = instanceGetter();
			if (instance.is_valid())
			{
				std::optional<SimValueT> value = property.get_value(instance).get_value<std::optional<SimValueT>>();

				OptionalProperty optionalProperty = qtProperty.value.value<OptionalProperty>();
				optionalProperty.property->setValue(simValueToQt(property, value.value_or(defaultValue)));
				optionalProperty.present = value.has_value();
				qtProperty.value.setValue(optionalProperty);
			}
		};
		r.updater(*r.property);

		if (!property.is_readonly())
		{
			r.applier = [instanceGetter, property] (const QtProperty& qtProperty) {
				rttr::instance instance = instanceGetter();
				if (instance.is_valid())
				{
					std::optional<SimValueT> value;

					OptionalProperty optionalProperty = qtProperty.value.value<OptionalProperty>();
					if (optionalProperty.present)
					{
						value = qtValueToSim<SimValueT>(property, optionalProperty.property->value);
					}
					property.set_value(instance, value);
				}
			};
		}

		return r;
	};
}

std::optional<QtPropertyUpdaterApplier> rttrPropertyToQt(const RttrInstanceGetter& instanceGetter, const rttr::property& property)
{
	const auto& type = property.get_type();

	static std::map<rttr::type::type_id, PropertyFactory> typePropertyFactories = {
		{ rttr::type::get<std::string>().get_id(), createPropertyFactory<std::string>("") },
		{ rttr::type::get<bool>().get_id(), createPropertyFactory<bool>(false) },
		{ rttr::type::get<int>().get_id(), createPropertyFactory<int>(0) },
		{ rttr::type::get<float>().get_id(), createPropertyFactory<float>(0.f) },
		{ rttr::type::get<double>().get_id(), createPropertyFactory<double>(0.0) },
		{ rttr::type::get<std::optional<std::string>>().get_id(), createPropertyFactory<std::string>("") },
		{ rttr::type::get<std::optional<bool>>().get_id(), createOptionalPropertyFactory<bool>(false) },
		{ rttr::type::get<std::optional<int>>().get_id(), createOptionalPropertyFactory<int>(0) },
		{ rttr::type::get<std::optional<float>>().get_id(), createOptionalPropertyFactory<float>(0.f) },
		{ rttr::type::get<std::optional<double>>().get_id(), createOptionalPropertyFactory<double>(0.0) },
		{ rttr::type::get<sim::Vector3>().get_id(), createPropertyFactory<sim::Vector3>(QVector3D(0,0,0)) },
		{ rttr::type::get<sim::Quaternion>().get_id(), createPropertyFactory<sim::Quaternion>(QVector3D(0,0,0)) },
		{ rttr::type::get<sim::LatLon>().get_id(), createPropertyFactory<sim::LatLon>(QVariant::fromValue(sim::LatLon(0,0))) }
	};

	if (const auto& i = typePropertyFactories.find(type.get_id()); i != typePropertyFactories.end())
	{
		return i->second(instanceGetter, property); 
	}
	return std::nullopt;
}

void addRttrPropertiesToModel(PropertiesModel& model, const rttr::array_range<rttr::property>& properties, const RttrInstanceGetter& instanceGetter)
{
	for (const rttr::property& property : properties)
	{
		std::optional<QtPropertyUpdaterApplier> qtProperty = rttrPropertyToQt(instanceGetter, property);
		if (qtProperty)
		{
			model.addProperty(qtProperty->property, qtProperty->updater, qtProperty->applier);
		}
	}
}