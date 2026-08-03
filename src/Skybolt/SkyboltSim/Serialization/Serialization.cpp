/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Serialization.h"
#include "SkyboltSim/JsonHelpers.h"
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Logging/Logging.h>

#include <format>

namespace skybolt::sim {

using ToReflVariantTranslator = std::function<refl::Instance(refl::TypeRegistry& registry, const nlohmann::json& propertyJson)>;

template <typename T>
ToReflVariantTranslator createToReflVariantTranslator()
{
	return [] (refl::TypeRegistry& registry, const nlohmann::json& json) {
		return refl::makeValueInstance(registry, json.get<T>());
		};
}

template <typename T>
ToReflVariantTranslator createOptionalToReflVariantTranslator()
{
	return [] (refl::TypeRegistry& registry, const nlohmann::json& json) {
		return refl::makeValueInstance(registry, std::optional<T>(json.get<T>()));
		};
}

static bool isSerializable(const refl::Property& property)
{
	return true;
}

static std::optional<refl::Instance> jsonToReflVariant(refl::TypeRegistry& registry, const refl::Type& type, const nlohmann::json& json)
{
	std::map<const refl::Type*, ToReflVariantTranslator> translators = {
		{ registry.getOrCreateType<bool>().get(), createToReflVariantTranslator<bool>() },
		{ registry.getOrCreateType<int>().get(), createToReflVariantTranslator<int>() },
		{ registry.getOrCreateType<unsigned int>().get(), createToReflVariantTranslator<unsigned int>() },
		{ registry.getOrCreateType<float>().get(), createToReflVariantTranslator<float>() },
		{ registry.getOrCreateType<double>().get(), createToReflVariantTranslator<double>() },
		{ registry.getOrCreateType<std::string>().get(), createToReflVariantTranslator<std::string>() },

		{ registry.getOrCreateType<std::optional<bool>>().get(), createOptionalToReflVariantTranslator<bool>() },
		{ registry.getOrCreateType<std::optional<int>>().get(), createOptionalToReflVariantTranslator<int>() },
		{ registry.getOrCreateType<std::optional<unsigned int>>().get(), createOptionalToReflVariantTranslator<unsigned int>() },
		{ registry.getOrCreateType<std::optional<float>>().get(), createOptionalToReflVariantTranslator<float>() },
		{ registry.getOrCreateType<std::optional<double>>().get(), createOptionalToReflVariantTranslator<double>() },
		{ registry.getOrCreateType<std::optional<std::string>>().get(), createOptionalToReflVariantTranslator<std::string>() },

		{ registry.getOrCreateType<sim::Vector3>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::makeValueInstance(registry, readVector3(json)); }},
		{ registry.getOrCreateType<sim::Quaternion>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::makeValueInstance(registry, readQuaternion(json)); }},
		{ registry.getOrCreateType<sim::LatLon>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::makeValueInstance(registry, readLatLon(json)); }},
		{ registry.getOrCreateType<sim::LatLonAlt>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::makeValueInstance(registry, readLatLonAlt(json)); }}
	};

	if (const auto& i = translators.find(&type); i != translators.end())
	{
		return (i->second)(registry, json);
	}
	return std::nullopt;
}

static void jsonToExistingReflVariant(refl::TypeRegistry& registry, refl::Instance& var, const nlohmann::json& json)
{
	if (const std::optional<refl::Instance>& newVar = jsonToReflVariant(registry, *var.getType(), json); newVar)
	{
		var = *newVar;
	}
	else
	{
		readReflectedObject(registry, var, json);
	}
}

static void readReflectedOptional(refl::TypeRegistry& registry, refl::Instance& object, refl::StdOptionalValueAccessor& accessor, const nlohmann::json& json)
{
	if (json.is_null())
	{
		accessor.setValues(object, {});
		return;
	}

	refl::TypePtr valueType = registry.getTypeByName(accessor.valueTypeName);
	if (!valueType)
	{
		throw std::runtime_error("Could not find type '" + accessor.valueTypeName + "' for optional deserialization");
	}

	// Create a default instance first so we have something to read into
	std::unique_ptr<refl::Instance> value = valueType->createDefaultInstance();
	if (!value)
	{
		throw std::runtime_error("Could not create default instance of type '" + accessor.valueTypeName + "' for optional deserialization");
	}

	// Now read into the instance we created
	jsonToExistingReflVariant(registry, *value, json);
	accessor.setValues(object, { *value });
}

static void readReflectedVector(refl::TypeRegistry& registry, refl::Instance& object, refl::StdVectorValueAccessor& accessor, const nlohmann::json& json)
{
	refl::TypePtr valueType = registry.getTypeByName(accessor.valueTypeName);
	if (!valueType)
	{
		throw std::runtime_error("Could not find type '" + accessor.valueTypeName + "' for vector deserialization");
	}

	std::vector<refl::Instance> values;
	for (const nlohmann::json& elementJson : json)
	{
		// Create a default instance first so we have something to read into
		std::unique_ptr<refl::Instance> value = valueType->createDefaultInstance();
		if (!value)
		{
			throw std::runtime_error("Could not create default instance of type '" + accessor.valueTypeName + "' for vector deserialization");
		}

		// Now read into the instance we created
		jsonToExistingReflVariant(registry, *value, elementJson);
		values.push_back(std::move(*value));
	}
	accessor.setValues(object, values);
}

void readReflectedObject(refl::TypeRegistry& registry, refl::Instance& object, const nlohmann::json& json)
{
	refl::TypePtr type = object.getType();
	if (type->isDerivedFrom<ExplicitSerialization>())
	{
		ExplicitSerialization& serialization = object.cast<ExplicitSerialization>();
		serialization.fromJson(registry, json);
	}
	else if (auto accessor = type->getContainerValueAccessor(); accessor)
	{
		if (auto optionalAccessor = dynamic_cast<refl::StdOptionalValueAccessor*>(accessor.get()); optionalAccessor)
		{
			readReflectedOptional(registry, object, *optionalAccessor, json);
		}
		else if (auto vectorAccessor = dynamic_cast<refl::StdVectorValueAccessor*>(accessor.get()); vectorAccessor)
		{
			readReflectedVector(registry, object, *vectorAccessor, json);
		}
	}
	else
	{
		readReflectedObjectProperties(registry, object, json);
	}
}

void readReflectedObjectProperties(refl::TypeRegistry& registry, refl::Instance& object, const nlohmann::json& json)
{
	for (const auto& [name, property] : getProperties(object))
	{
		try
		{
			if (isSerializable(*property))
			{
				ifChildExists(json, property->getName(), [&](const nlohmann::json& propertyJson) {
					refl::Instance value = property->getValue(object);
					jsonToExistingReflVariant(registry, value, propertyJson);
					property->setValue(object, value);
					});
			}
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(std::format("Failed to deserialize property \"{}\" of type \"{}\": {}", property->getName(), property->getType()->getName(), e.what()));
		}
	}
}

using ToJsonTranslator = std::function<nlohmann::json(const refl::Instance& var)>;

template <typename T>
ToJsonTranslator createToJsonTranslator()
{
	return [] (const refl::Instance& var) {
		return nlohmann::json(var.cast<T>());
	};
}

template <typename T>
ToJsonTranslator createOptionalToJsonTranslator()
{
	return [] (const refl::Instance& var) {
		auto value = var.cast<std::optional<T>>();
		return value ? nlohmann::json(*value) : nlohmann::json();
	};
}

static nlohmann::json toJson(refl::TypeRegistry& registry, const refl::Type& type, const refl::Instance& var)
{
	std::map<const refl::Type*, ToJsonTranslator> translators = {
		{ registry.getOrCreateType<bool>().get(), createToJsonTranslator<bool>() },
		{ registry.getOrCreateType<int>().get(), createToJsonTranslator<int>() },
		{ registry.getOrCreateType<unsigned int>().get(), createToJsonTranslator<unsigned int>() },
		{ registry.getOrCreateType<float>().get(), createToJsonTranslator<float>() },
		{ registry.getOrCreateType<double>().get(), createToJsonTranslator<double>() },
		{ registry.getOrCreateType<std::string>().get(), createToJsonTranslator<std::string>() },

		{ registry.getOrCreateType<std::optional<bool>>().get(), createOptionalToJsonTranslator<bool>() },
		{ registry.getOrCreateType<std::optional<int>>().get(), createOptionalToJsonTranslator<int>() },
		{ registry.getOrCreateType<std::optional<unsigned int>>().get(), createOptionalToJsonTranslator<unsigned int>() },
		{ registry.getOrCreateType<std::optional<float>>().get(), createOptionalToJsonTranslator<float>() },
		{ registry.getOrCreateType<std::optional<double>>().get(), createOptionalToJsonTranslator<double>() },
		{ registry.getOrCreateType<std::optional<std::string>>().get(), createOptionalToJsonTranslator<std::string>() },

		{ registry.getOrCreateType<sim::Vector3>().get(), [] (const refl::Instance& var) {	return writeJson(var.cast<sim::Vector3>()); }},
		{ registry.getOrCreateType<sim::Quaternion>().get(), [] (const refl::Instance& var) { return writeJson(var.cast<sim::Quaternion>()); }},
		{ registry.getOrCreateType<sim::LatLon>().get(), [] (const refl::Instance& var) { return writeJson(var.cast<sim::LatLon>()); }},
		{ registry.getOrCreateType<sim::LatLonAlt>().get(), [] (const refl::Instance& var) { return writeJson(var.cast<sim::LatLonAlt>()); }}
	};

	if (const auto& i = translators.find(&type); i != translators.end())
	{
		return (i->second)(var);
	}
	else
	{
		return writeReflectedObject(registry, var);
	}

	return {};
}

static nlohmann::json writeReflectedOptional(refl::TypeRegistry& registry, const refl::Instance& object, refl::StdOptionalValueAccessor& accessor)
{
	std::vector<refl::Instance> values = accessor.getValues(registry, object);
	if (values.empty())
	{
		return nlohmann::json(); // null json for empty optional
	}
	const refl::Instance& value = values.front();
	return toJson(registry, *value.getType(), value);
}

static nlohmann::json writeReflectedVector(refl::TypeRegistry& registry, const refl::Instance& object, refl::StdVectorValueAccessor& accessor)
{
	nlohmann::json json = nlohmann::json::array();
	for (const refl::Instance& value : accessor.getValues(registry, object))
	{
		json.push_back(toJson(registry, *value.getType(), value));
	}
	return json;
}

nlohmann::json writeReflectedObject(refl::TypeRegistry& registry, const refl::Instance& object)
{
	nlohmann::json json;

	const auto& type = object.getType();

	// Write using explicit serialization implementation if it exists
	if (type->isDerivedFrom<ExplicitSerialization>())
	{
		const ExplicitSerialization& serialization = object.cast<ExplicitSerialization>();
		json = serialization.toJson(registry);
		return json;
	}
	
	// Fallback to reflection based serialization
	if (auto accessor = type->getContainerValueAccessor(); accessor)
	{
		if (auto optionalValueTranslator = dynamic_cast<refl::StdOptionalValueAccessor*>(accessor.get()); optionalValueTranslator)
		{
			return writeReflectedOptional(registry, object, *optionalValueTranslator);
		}
		else if (auto vectorValueTranslator = dynamic_cast<refl::StdVectorValueAccessor*>(accessor.get()); vectorValueTranslator)
		{
			return writeReflectedVector(registry, object, *vectorValueTranslator);
		}
	}

	return writeReflectedObjectProperties(registry, object);
}

bool isNan(const nlohmann::json& json)
{
	return json.is_number() && std::isnan(json.get<double>());
}

nlohmann::json writeReflectedObjectProperties(refl::TypeRegistry& registry, const refl::Instance& object)
{
	nlohmann::json json;

	for (const auto& [name, property] : getProperties(object))
	{
		if (isSerializable(*property))
		{
			auto type = property->getType();
			refl::Instance var = property->getValue(object);
			nlohmann::json valueJson = toJson(registry, *property->getType(), var);
			if (!valueJson.is_null())
			{
				if (isNan(valueJson))
				{
					// NaN values can't be represented in JSON, so represent them as 0.0 and log an error.
					json[property->getName()] = 0.0;
					SKYBOLT_LOG(error) << std::format("Property \"{}\" of type \"{}\" has NaN value which can't be serialized to JSON. Representing it as 0.0 in JSON.", property->getName(), property->getType()->getName());
				}
				else
				{
					json[property->getName()] = valueJson;
				}
			}
		}
	}

	return json;
}

} // namespace skybolt::sim