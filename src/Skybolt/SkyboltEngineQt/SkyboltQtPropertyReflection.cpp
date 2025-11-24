#include "SkyboltQtPropertyReflection.h"
#include "QtTypeConversions.h"

#include <SkyboltCommon/Units.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltEngine/Components/VisObjectsComponent.h>
#include <SkyboltWidgets/Util/QtTypeConversions.h>
#include <SkyboltWidgets/Property/QtPropertyReflectionConversion.h>
#include <SkyboltReflect/Reflection.h>
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

namespace skybolt {
	// Add conversion templates for custom types

	template <>
	QVariant reflValueToQt(const refl::Property& property, const float& value)
	{
		return simUnitToDisplay(property, value);
	}

	template <>
	float qtValueToRefl(const refl::Property& property, const QVariant& value)
	{
		return displayUnitToSim(property, value.toFloat());
	}

	template <>
	QVariant reflValueToQt(const refl::Property& property, const double& value)
	{
		return simUnitToDisplay(property, value);
	}

	template <>
	double qtValueToRefl(const refl::Property& property, const QVariant& value)
	{
		return displayUnitToSim(property, value.toDouble());
	}

	template <>
	QVariant reflValueToQt(const refl::Property& property, const sim::Vector3& value)
	{
		return toQVector3D(value);
	}

	template <>
	sim::Vector3 qtValueToRefl(const refl::Property& property, const QVariant& value)
	{
		return toVector3(value.value<QVector3D>());
	}

	template <>
	QVariant reflValueToQt(const refl::Property& property, const sim::Quaternion& value)
	{
		glm::dvec3 euler = skybolt::math::eulerFromQuat(value) * skybolt::math::radToDegD();
		return QVector3D(euler.x, euler.y, euler.z);
	}

	template <>
	sim::Quaternion qtValueToRefl(const refl::Property& property, const QVariant& value)
	{
		sim::Vector3 euler = toVector3(value.value<QVector3D>());
		return skybolt::math::quatFromEuler(euler * skybolt::math::degToRadD());
	}

	template <>
	QVariant reflValueToQt(const refl::Property& property, const sim::LatLon& value)
	{
		return QVariant::fromValue(sim::LatLon(value.lat * skybolt::math::radToDegD(), value.lon * skybolt::math::radToDegD()));
	}

	template <>
	sim::LatLon qtValueToRefl(const refl::Property& property, const QVariant& value)
	{
		sim::LatLon simValue = value.value<sim::LatLon>();
		simValue.lat *= skybolt::math::degToRadD();
		simValue.lon *= skybolt::math::degToRadD();
		return simValue;
	}

} // namespace skybolt

skybolt::ReflTypePropertyFactoryMap createSkyboltReflTypePropertyFactories(skybolt::refl::TypeRegistry& typeRegistry)
{
	skybolt::ReflTypePropertyFactoryMap factories = createDefaultReflTypePropertyFactories(typeRegistry);

	factories[typeRegistry.getOrCreateType<float>()] = createPropertyFactory<float>(0.f); // Override the default float and double property factories to enable units conversion
	factories[typeRegistry.getOrCreateType<double>()] = createPropertyFactory<double>(0.0);
	factories[typeRegistry.getOrCreateType<sim::Vector3>()] = createPropertyFactory<sim::Vector3>(QVector3D(0,0,0));
	factories[typeRegistry.getOrCreateType<sim::Quaternion>()] = createPropertyFactory<sim::Quaternion>(QVector3D(0,0,0));
	factories[typeRegistry.getOrCreateType<sim::LatLon>()] = createPropertyFactory<sim::LatLon>(QVariant::fromValue(sim::LatLon(0,0)));
	return factories;
}
