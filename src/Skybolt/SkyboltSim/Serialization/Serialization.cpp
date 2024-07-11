/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Serialization.h"
#include "SkyboltSim/JsonHelpers.h"
#include <SkyboltCommon/Json/JsonHelpers.h>

#include <boost/log/trivial.hpp>

namespace skybolt::sim {

using ToReflVariantTranslator = std::function<refl::Instance(refl::TypeRegistry& registry, const nlohmann::json& propertyJson)>;

template <typename T>
ToReflVariantTranslator createToReflVariantTranslator()
{
	return [] (refl::TypeRegistry& registry, const nlohmann::json& json) {
		return refl::createOwningInstance(&registry, json.get<T>());
		};
}

template <typename T>
ToReflVariantTranslator createOptionalToReflVariantTranslator()
{
	return [] (refl::TypeRegistry& registry, const nlohmann::json& json) {
		return refl::createOwningInstance(&registry, std::optional<T>(json.get<T>()));
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
		{ registry.getOrCreateType<std::optional<std::string>>().get(), createToReflVariantTranslator<std::string>() },

		{ registry.getOrCreateType<sim::Vector3>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::createOwningInstance(&registry, readVector3(json)); }},
		{ registry.getOrCreateType<sim::Quaternion>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::createOwningInstance(&registry, readQuaternion(json)); }},
		{ registry.getOrCreateType<sim::LatLon>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::createOwningInstance(&registry, readLatLon(json)); }},
		{ registry.getOrCreateType<sim::LatLonAlt>().get(), [] (refl::TypeRegistry& registry, const nlohmann::json& json) { return refl::createOwningInstance(&registry, readLatLonAlt(json)); }}
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

void readReflectedObject(refl::TypeRegistry& registry, refl::Instance& object, const nlohmann::json& json)
{
	refl::TypePtr type = object.getType();
	if (type->isDerivedFrom<ExplicitSerialization>())
	{
		if (ExplicitSerialization* serialization = object.getObject<ExplicitSerialization>(); serialization)
		{
			serialization->fromJson(registry, json);
		}
		else
		{
			assert(!"Should not get here");
		}
	}
	else // use reflection based serialization
	{
		for (const auto& [name, property] : type->getProperties())
		{
			if (isSerializable(*property))
			{
				ifChildExists(json, property->getName(), [&] (const nlohmann::json& propertyJson) {
					refl::Instance value = property->getValue(object);
					jsonToExistingReflVariant(registry, value, propertyJson);
					property->setValue(object, value);
				});
			}
		}
	}
}

using ToJsonTranslator = std::function<nlohmann::json(const refl::Instance& var)>;

template <typename T>
ToJsonTranslator createToJsonTranslator()
{
	return [] (const refl::Instance& var) {
		return nlohmann::json(*var.getObject<T>());
	};
}

template <typename T>
ToJsonTranslator createOptionalToJsonTranslator()
{
	return [] (const refl::Instance& var) {
		auto value = *var.getObject<std::optional<T>>();
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

		{ registry.getOrCreateType<sim::Vector3>().get(), [] (const refl::Instance& var) {	return writeJson(*var.getObject<sim::Vector3>()); }},
		{ registry.getOrCreateType<sim::Quaternion>().get(), [] (const refl::Instance& var) { return writeJson(*var.getObject<sim::Quaternion>()); }},
		{ registry.getOrCreateType<sim::LatLon>().get(), [] (const refl::Instance& var) { return writeJson(*var.getObject<sim::LatLon>()); }},
		{ registry.getOrCreateType<sim::LatLonAlt>().get(), [] (const refl::Instance& var) { return writeJson(*var.getObject<sim::LatLonAlt>()); }}
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

nlohmann::json writeReflectedObject(refl::TypeRegistry& registry, const refl::Instance& object)
{
	nlohmann::json json;

	const auto& type = object.getType();

	if (type->isDerivedFrom<ExplicitSerialization>())
	{
		if (const ExplicitSerialization* serialization = object.getObject<ExplicitSerialization>(); serialization)
		{
			json = serialization->toJson(registry);
		}
		else
		{
			assert(!"Should not get here");
		}
	}
	else // use reflection based serialization
	{
		for (const auto& [name, property] : type->getProperties())
		{
			if (isSerializable(*property))
			{
				auto type = property->getType();
				refl::Instance var = property->getValue(object);
				nlohmann::json valueJson = toJson(registry, *property->getType(), var);
				if (!valueJson.is_null())
				{
					json[property->getName()] = valueJson;
				}
			}
		}
	}

	return json;
}

} // namespace skybolt::sim