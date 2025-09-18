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
#include <SkyboltSim/Components/AttacherComponent.h>

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

template <typename SimValueT>
static QVariant valueInstanceToQt(const refl::Property& property, const refl::Instance& valueInstance)
{
	if constexpr (std::is_same_v<SimValueT, ReflPropertyInstanceVariant>)
	{
		return QVariant::fromValue(ReflPropertyInstanceVariant{ valueInstance });
	}
	else
	{
		SimValueT value = *valueInstance.getObject<SimValueT>();
		return simValueToQt(property, value);
	}
}

template <typename SimValueT>
static std::optional<refl::Instance> qtToValueInstance(const refl::Property& property, refl::TypeRegistry& typeRegistry, const QVariant value, const QVariant& defaultValue)
{
	if constexpr (std::is_same_v<SimValueT, ReflPropertyInstanceVariant>)
	{
		std::optional<refl::Instance> optionalValueInstance = value.value<ReflPropertyInstanceVariant>().valueInstance;
		if (!optionalValueInstance)
		{
			optionalValueInstance = defaultValue.value<ReflPropertyInstanceVariant>().instance;
		}
		return optionalValueInstance;
	}
	else
	{
		SimValueT simValue = qtValueToSim<SimValueT>(property, value);
		return refl::createOwningInstance(typeRegistry, simValue);
	}
}

using PropertyFactory = std::function<QtPropertyUpdaterApplier(refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property)>;

template <typename SimValueT, typename QtValueT>
static QtPropertyUpdaterApplier createValueProperty(refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property, const QtValueT& defaultValue)
{
	QString name = QString::fromStdString(property->getName());
	QVariant defaultValueVariant = QVariant::fromValue(defaultValue);

	QtPropertyUpdaterApplier r;
	r.property = createQtProperty(name, defaultValueVariant);
	r.property->enabled = !property->isReadOnly();
	addMetadata(*r.property, *property);
		
	r.updater = [instanceGetter, property] (QtProperty& qtProperty) {
		if (const auto& objectInstance = instanceGetter(); objectInstance)
		{
			refl::Instance valueInstance = property->getValue(*objectInstance);
			qtProperty.setValue(valueInstanceToQt<SimValueT>(*property, valueInstance));
		}
	};
	r.updater(*r.property);

	if (!property->isReadOnly())
	{
		r.applier = [typeRegistry = &typeRegistry, instanceGetter, property, defaultValueVariant] (const QtProperty& qtProperty) {
			if (auto objectInstance = instanceGetter(); objectInstance)
			{
				std::optional<refl::Instance> valueInstance = qtToValueInstance<SimValueT>(*property, *typeRegistry, qtProperty.value(), defaultValueVariant);
				if (valueInstance)
				{
					property->setValue(*objectInstance, *valueInstance);
				}
			}
		};
	}

	return r;
}

static std::optional<refl::Instance> getOptionalValue(refl::TypeRegistry& registry, const refl::Instance& optionalInstance)
{
	auto accessor = optionalInstance.getType()->getContainerValueAccessor();
	if (!accessor) { return std::nullopt; }

	auto values = accessor->getValues(registry, optionalInstance);
	return values.empty() ? std::optional<refl::Instance>() : values.front();
}

static void setOptionalValue(refl::Instance& optionalInstance, const std::optional<refl::Instance>& value)
{
	auto accessor = optionalInstance.getType()->getContainerValueAccessor();
	if (!accessor) { return; }

	accessor->setValues(optionalInstance, value ? std::vector<refl::Instance>({*value}) : std::vector<refl::Instance>());
}

template <typename SimValueT, typename QtValueT>
static QtPropertyUpdaterApplier createOptionalValueProperty(refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property, const QtValueT& defaultValue)
{
	QString name = QString::fromStdString(property->getName());
	QVariant defaultValueVariant = QVariant::fromValue(defaultValue);
	QtPropertyPtr childProperty = createQtProperty(name, defaultValueVariant);
	addMetadata(*childProperty, *property);

	OptionalProperty optionalProperty;
	optionalProperty.property = childProperty;

	QtPropertyUpdaterApplier r;
	r.property = createQtProperty(name, QVariant::fromValue(optionalProperty));
	r.property->enabled = !property->isReadOnly();

	auto propagateChangeSignalToParent = std::make_shared<bool>(true);
	QObject::connect(childProperty.get(), &QtProperty::valueChanged, r.property.get(), [propagateChangeSignalToParent, parentProperty = r.property.get()] {
		if (*propagateChangeSignalToParent) { parentProperty->valueChanged(); }
		});

	r.updater = [typeRegistry = &typeRegistry, instanceGetter, property, propagateChangeSignalToParent] (QtProperty& qtProperty) {
		if (const auto& objectInstance = instanceGetter(); objectInstance)
		{
			refl::Instance optionalInstance = property->getValue(*objectInstance);
			std::optional<refl::Instance> valueInstance = getOptionalValue(*typeRegistry, optionalInstance);

			OptionalProperty optionalProperty = qtProperty.value().value<OptionalProperty>();

			if (valueInstance)
			{
				// Update the child property witout propagating update signal to parent
				// to avoid unnecessary update, as we will be updating the parent after.
				*propagateChangeSignalToParent = false;
				QVariant qtValue = valueInstanceToQt<SimValueT>(*property, *valueInstance);
				optionalProperty.property->setValue(qtValue);
				*propagateChangeSignalToParent = true;
			}

			optionalProperty.present = valueInstance.has_value();
			qtProperty.setValue(QVariant::fromValue(optionalProperty));
		}
	};
	r.updater(*r.property);

	if (!property->isReadOnly())
	{
		r.applier = [typeRegistry = &typeRegistry, instanceGetter, property, defaultValue] (const QtProperty& qtProperty) {
			if (auto objectInstance = instanceGetter(); objectInstance)
			{
				std::optional<refl::Instance> valueInstance;

				OptionalProperty optionalProperty = qtProperty.value().value<OptionalProperty>();
				if (optionalProperty.present)
				{
					assert(optionalProperty.property);
					valueInstance = qtToValueInstance<SimValueT>(*property, *typeRegistry, optionalProperty.property->value(), defaultValue);
				}
					
				refl::Instance optionalInstance = property->getValue(*objectInstance);
				setOptionalValue(optionalInstance, valueInstance);
				property->setValue(*objectInstance, optionalInstance);
			}
		};
	}

	return r;
}

static std::vector<refl::Instance> getVectorValues(refl::TypeRegistry& registry, const refl::Instance& vectorInstance)
{
	auto accessor = vectorInstance.getType()->getContainerValueAccessor();
	if (!accessor) { return {}; }

	return accessor->getValues(registry, vectorInstance);
}

static void setVectorValues(refl::Instance& vectorInstance, const std::vector<refl::Instance>& values)
{
	auto accessor = vectorInstance.getType()->getContainerValueAccessor();
	if (!accessor) { return; }

	accessor->setValues(vectorInstance, values);
}

template <typename SimValueT, typename QtValueT>
static QtPropertyUpdaterApplier createVectorValueProperty(refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property, const QtValueT& defaultValue)
{
	QString name = QString::fromStdString(property->getName());
	QVariant defaultValueVariant = QVariant::fromValue(defaultValue);

	PropertyVector propertyVector;
	propertyVector.itemDefaultValue = defaultValueVariant;

	QtPropertyUpdaterApplier r;
	r.property = createQtProperty(name, QVariant::fromValue(propertyVector));
	r.property->enabled = !property->isReadOnly();

	r.updater = [typeRegistry = &typeRegistry, instanceGetter, property](QtProperty& qtProperty) {
		if (const auto& objectInstance = instanceGetter(); objectInstance)
		{
			refl::Instance vectorInstance = property->getValue(*objectInstance);
			std::vector<refl::Instance> valueInstances = getVectorValues(*typeRegistry, vectorInstance);

			PropertyVector propertyVector = qtProperty.value().value<PropertyVector>();
			propertyVector.items.reserve(valueInstances.size());
			propertyVector.items.clear();
			
			int i = 0;
			for (const auto& valueInstance : valueInstances)
			{
				QString name = QString::number(i);
				QVariant qtValue = valueInstanceToQt<SimValueT>(*property, valueInstance);

				QtPropertyPtr itemProperty = createQtProperty(name, qtValue);
				addMetadata(*itemProperty, *property);
				QObject::connect(itemProperty.get(), &QtProperty::valueChanged, &qtProperty, &QtProperty::valueChanged);
				propertyVector.items.push_back(itemProperty);
				++i;
			}

			qtProperty.setValue(QVariant::fromValue(propertyVector));
		}
	};
	r.updater(*r.property);

	if (!property->isReadOnly())
	{
		r.applier = [typeRegistry = &typeRegistry, instanceGetter, property, defaultValueVariant](const QtProperty& qtProperty) {
			if (auto objectInstance = instanceGetter(); objectInstance)
			{
				PropertyVector propertyVector = qtProperty.value().value<PropertyVector>();

				std::vector<refl::Instance> valueInstances;
				valueInstances.reserve(propertyVector.items.size());

				for (const auto& valueProperty : propertyVector.items)
				{
					assert(valueProperty);
					std::optional<refl::Instance> valueInstance = qtToValueInstance<SimValueT>(*property, *typeRegistry, valueProperty->value(), defaultValueVariant);
					if (valueInstance)
					{
						valueInstances.push_back(*valueInstance);
					}
				}

				refl::Instance vectorInstance = property->getValue(*objectInstance);
				setVectorValues(vectorInstance, valueInstances);
				property->setValue(*objectInstance, vectorInstance);
			}
		};
	}

	return r;
}

template <typename SimValueT, typename QtValueT>
PropertyFactory createPropertyFactory(const QtValueT& defaultValue)
{
	return [defaultValue] (refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property) {	
		refl::TypePtr type = property->getType();
		auto accessor = type->getContainerValueAccessor();
		
		if (accessor)
		{
			if (auto optionalValueAccessor = dynamic_cast<refl::StdOptionalValueAccessor*>(accessor.get()); optionalValueAccessor)
			{
				 return createOptionalValueProperty<SimValueT, QtValueT>(typeRegistry, instanceGetter, property, defaultValue);
			}
			else if (auto vectorValueAccessor = dynamic_cast<refl::StdVectorValueAccessor*>(accessor.get()); vectorValueAccessor)
			{
				return createVectorValueProperty<SimValueT, QtValueT>(typeRegistry, instanceGetter, property, defaultValue);
			}
		}

		return createValueProperty<SimValueT, QtValueT>(typeRegistry, instanceGetter, property, defaultValue);
	};
}

std::optional<QtPropertyUpdaterApplier> reflPropertyToQt(refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const refl::PropertyPtr& property)
{
	auto type = property->getType();
	if (auto accessor = type->getContainerValueAccessor(); accessor)
	{
		type = typeRegistry.getTypeByName(accessor->valueTypeName);
		if (!type)
		{
			return std::nullopt;
		}
	}

	std::map<refl::TypePtr, PropertyFactory> typePropertyFactories = {
		{ typeRegistry.getOrCreateType<std::string>(), createPropertyFactory<std::string>(QString()) },
		{ typeRegistry.getOrCreateType<bool>(), createPropertyFactory<bool>(false) },
		{ typeRegistry.getOrCreateType<int>(), createPropertyFactory<int>(0) },
		{ typeRegistry.getOrCreateType<float>(), createPropertyFactory<float>(0.f) },
		{ typeRegistry.getOrCreateType<double>(), createPropertyFactory<double>(0.0) },
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
			model.addProperty(qtProperty->property, qtProperty->updater, qtProperty->applier, property->getCategory());
		}
	}
}