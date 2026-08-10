/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SkyboltQtPropertyReflection.h"
#include "QtTypeConversions.h"

#include <SkyboltCommon/Units.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltSim/CameraController/CameraModifier.h>
#include <SkyboltSim/PropertyMetadata.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltVisOsg/Components/VisObjectsComponent.h>
#include <SkyboltWidgets/Util/QtTypeConversions.h>
#include <SkyboltWidgets/Property/QtPropertyReflectionConversion.h>

#include <QVector3D>

using namespace skybolt;

namespace skybolt {
	// Add conversion templates for custom types

	template <>
	QVariant reflValueToQt(const sim::Vector3& value)
	{
		return toQVector3D(value);
	}

	template <>
	sim::Vector3 qtValueToRefl(const QVariant& value)
	{
		return toVector3(value.value<QVector3D>());
	}

	template <>
	QVariant reflValueToQt(const sim::Quaternion& value)
	{
		glm::dvec3 euler = skybolt::math::eulerFromQuat(value) * skybolt::math::radToDegD();
		return QVector3D(euler.x, euler.y, euler.z);
	}

	template <>
	sim::Quaternion qtValueToRefl(const QVariant& value)
	{
		sim::Vector3 euler = toVector3(value.value<QVector3D>());
		return skybolt::math::quatFromEuler(euler * skybolt::math::degToRadD());
	}

	template <>
	QVariant reflValueToQt(const sim::LatLon& value)
	{
		return QVariant::fromValue(sim::LatLon(value.lat * skybolt::math::radToDegD(), value.lon * skybolt::math::radToDegD()));
	}

	template <>
	sim::LatLon qtValueToRefl(const QVariant& value)
	{
		sim::LatLon simValue = value.value<sim::LatLon>();
		simValue.lat *= skybolt::math::degToRadD();
		simValue.lon *= skybolt::math::degToRadD();
		return simValue;
	}

} // namespace skybolt

skybolt::ReflValueTranslatorMap createSkyboltReflValueTranslators(skybolt::refl::TypeRegistry& typeRegistry)
{
	skybolt::ReflValueTranslatorMap factories = createDefaultReflValueTranslators(typeRegistry);

	factories[typeRegistry.getOrCreateType<sim::Vector3>()] = createReflValueTranslator<sim::Vector3, QVector3D>();
	factories[typeRegistry.getOrCreateType<sim::Quaternion>()] = createReflValueTranslator<sim::Quaternion, QVector3D>();
	factories[typeRegistry.getOrCreateType<sim::LatLon>()] = createReflValueTranslator<sim::LatLon, QVariant>();
	factories[typeRegistry.getOrCreateType<skybolt::sim::CameraControllerSelector::ControllersMap>()] = createReflValueTranslator<skybolt::sim::CameraControllerSelector::ControllersMap, QVariant>();
	factories[typeRegistry.getOrCreateType<skybolt::sim::CameraModifierPtr>()] = createReflValueTranslatorWithDisplayName<skybolt::sim::CameraModifierPtr, QVariant>(/* displayNameRenderer */[] (const refl::Instance& reflValue, const QVariant& qtValue) {
		auto cameraModifier = reflValue.cast<skybolt::sim::CameraModifierPtr>();
		return cameraModifier ? QString::fromStdString(cameraModifier->getTypeName()) : "null";
	});
	return factories;
}
