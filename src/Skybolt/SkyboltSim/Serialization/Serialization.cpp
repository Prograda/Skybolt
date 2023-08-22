/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Serialization.h"
#include "SkyboltSim/JsonHelpers.h"
#include <SkyboltCommon/Json/JsonHelpers.h>

#include <boost/log/trivial.hpp>

namespace skybolt::sim {

using ToRttrVariantTranslator = std::function<rttr::variant(const nlohmann::json& propertyJson)>;

template <typename T>
ToRttrVariantTranslator createToRttrVariantTranslator()
{
	return [] (const nlohmann::json& json) {
		return json.get<T>();
		};
}

template <typename T>
ToRttrVariantTranslator createOptionalToRttrVariantTranslator()
{
	return [] (const nlohmann::json& json) {
		return std::optional<T>(json.get<T>());
		};
}

static bool isSerializable(const rttr::property& property)
{
	return true;
}

static void jsonToSequentialContainer(rttr::variant& var, const nlohmann::json& json);

static void jsonToAssociativeContainer(rttr::variant& var, const nlohmann::json& json);

rttr::variant jsonToRttrVariant(const rttr::type& type, const nlohmann::json& json)
{
	static std::map<rttr::type::type_id, ToRttrVariantTranslator> translators = {
		{ rttr::type::get<bool>().get_id(), createToRttrVariantTranslator<bool>() },
		{ rttr::type::get<int>().get_id(), createToRttrVariantTranslator<int>() },
		{ rttr::type::get<float>().get_id(), createToRttrVariantTranslator<float>() },
		{ rttr::type::get<double>().get_id(), createToRttrVariantTranslator<double>() },
		{ rttr::type::get<std::string>().get_id(), createToRttrVariantTranslator<std::string>() },

		{ rttr::type::get<std::optional<bool>>().get_id(), createOptionalToRttrVariantTranslator<bool>() },
		{ rttr::type::get<std::optional<int>>().get_id(), createOptionalToRttrVariantTranslator<int>() },
		{ rttr::type::get<std::optional<float>>().get_id(), createOptionalToRttrVariantTranslator<float>() },
		{ rttr::type::get<std::optional<double>>().get_id(), createOptionalToRttrVariantTranslator<double>() },
		{ rttr::type::get<std::optional<std::string>>().get_id(), createToRttrVariantTranslator<std::string>() },

		{ rttr::type::get<sim::Vector3>().get_id(), [] (const nlohmann::json& json) { return readVector3(json); }},
		{ rttr::type::get<sim::Quaternion>().get_id(), [] (const nlohmann::json& json) { return readQuaternion(json); }},
		{ rttr::type::get<sim::LatLon>().get_id(), [] (const nlohmann::json& json) { return readLatLon(json); }},
		{ rttr::type::get<sim::LatLonAlt>().get_id(), [] (const nlohmann::json& json) { return readLatLonAlt(json); }}
	};

	if (const auto& i = translators.find(type.get_id()); i != translators.end())
	{
		return (i->second)(json);
	}
	return {};
}

//! Returns the value wrapped by ref_wrapper, shared_ptr, unique_ptr
static rttr::variant unwrap(const rttr::variant& var)
{
	rttr::variant unwrappedVar = var;
	auto type = var.get_type();

	// Loop to handle multiple levels of wrapping, This is required when unwrapping values in map<Key, shared_ptr<Object>>
	// because RTTR wraps the values in a ref_wrapper, i.e ref_wrapper<shared_ptr<Object>>.
	while (type.is_wrapper())
	{
		type = type.get_wrapped_type();
		unwrappedVar = unwrappedVar.extract_wrapped_value();
	}
	return unwrappedVar;
}

void jsonToExistingRttrVariant(rttr::variant& var, const nlohmann::json& json)
{
	assert(var.is_valid());

	rttr::variant unwrappedVar = unwrap(var);
	const auto& type = unwrappedVar.get_type();

	if (const rttr::variant& newVar = jsonToRttrVariant(unwrappedVar.get_type(), json); newVar.is_valid())
	{
		unwrappedVar = newVar;
	}
	else if (type.is_sequential_container())
	{
		jsonToSequentialContainer(unwrappedVar, json);
	}
	else if (type.is_associative_container())
	{
		jsonToAssociativeContainer(unwrappedVar, json);
	}
	else
	{
		readReflectedObject(rttr::instance(unwrappedVar), json);
	}
	var = unwrappedVar; // RTTI automatically re-wraps the value as needed
}

static void jsonToSequentialContainer(rttr::variant& var, const nlohmann::json& json)
{
	rttr::variant_sequential_view view = var.create_sequential_view();
	int i = 0;
	for (const auto& jsonIt : json.items())
	{
		rttr::variant value;
		if (i < view.get_size())
		{
			value = unwrap(view.get_value(i));
			jsonToExistingRttrVariant(value, jsonIt.value());
		}
		else
		{
			view.set_size(i+1);
			value = jsonToRttrVariant(view.get_value_type(), jsonIt.value());
		}

		if (value.is_valid())
		{
			view.set_value(i, value);
		}
		else
		{
			BOOST_LOG_TRIVIAL(warning) << "Could not read value '" << jsonIt.value() << "' of type '" << var.get_type().get_name().to_string() << "'from JSON";
		}
		++i;
	}
}

static void jsonToAssociativeContainer(rttr::variant& var, const nlohmann::json& json)
{
	rttr::variant_associative_view view = var.create_associative_view();
	for (const auto& jsonIt : json.items())
	{
		rttr::variant value;
		if (auto viewIt = view.find(jsonIt.key()); viewIt != view.end())
		{
			value = unwrap(viewIt.get_value());
			jsonToExistingRttrVariant(value, jsonIt.value());
		}
		else
		{
			value = jsonToRttrVariant(view.get_value_type(), jsonIt.value());
		}

		if (value.is_valid())
		{
			view.insert(jsonIt.key(), value);
		}
		else
		{
			BOOST_LOG_TRIVIAL(warning) << "Could not read value '" << jsonIt.value() << "' of type '" << var.get_type().get_name().to_string() << "'from JSON";
		}
	}
}

void readReflectedObject(rttr::instance& object, const nlohmann::json& json)
{
	rttr::type type = object.get_derived_type();
	if (type.is_derived_from<ExplicitSerialization>())
	{
		if (ExplicitSerialization* serialization = object.try_convert<ExplicitSerialization>(); serialization)
		{
			serialization->fromJson(json);
		}
		else
		{
			assert(!"Should not get here");
		}
	}
	else // use reflection based serialization
	{
		const auto& props = type.get_properties();
		for (const rttr::property& property : props)
		{
			if (isSerializable(property))
			{
				ifChildExists(json, property.get_name().to_string(), [&] (const nlohmann::json& propertyJson) {
					rttr::variant& var = property.get_value(object);
					jsonToExistingRttrVariant(var, propertyJson);
					if (var.is_valid())
					{
						property.set_value(object, var);
					}
				});
			}
		}
	}
}

using ToJsonTranslator = std::function<nlohmann::json(const rttr::variant& var)>;

template <typename T>
ToJsonTranslator createToJsonTranslator()
{
	return [] (const rttr::variant& var) { return nlohmann::json(var.get_value<T>()); };
}

template <typename T>
ToJsonTranslator createOptionalToJsonTranslator()
{
	return [] (const rttr::variant& var) {
		auto value = var.get_value<std::optional<T>>();
		return value ? nlohmann::json(*value) : nlohmann::json();
	};
}

static nlohmann::json sequentialContainerToJson(const rttr::variant& var);

static nlohmann::json associativeContainerToJson(const rttr::variant& var);

static nlohmann::json toJson(const rttr::variant& var)
{
	static std::map<rttr::type::type_id, ToJsonTranslator> translators = {
		{ rttr::type::get<bool>().get_id(), createToJsonTranslator<bool>() },
		{ rttr::type::get<int>().get_id(), createToJsonTranslator<int>() },
		{ rttr::type::get<float>().get_id(), createToJsonTranslator<float>() },
		{ rttr::type::get<double>().get_id(), createToJsonTranslator<double>() },
		{ rttr::type::get<std::string>().get_id(), createToJsonTranslator<std::string>() },

		{ rttr::type::get<std::optional<bool>>().get_id(), createOptionalToJsonTranslator<bool>() },
		{ rttr::type::get<std::optional<int>>().get_id(), createOptionalToJsonTranslator<int>() },
		{ rttr::type::get<std::optional<float>>().get_id(), createOptionalToJsonTranslator<float>() },
		{ rttr::type::get<std::optional<double>>().get_id(), createOptionalToJsonTranslator<double>() },
		{ rttr::type::get<std::optional<std::string>>().get_id(), createOptionalToJsonTranslator<std::string>() },

		{ rttr::type::get<sim::Vector3>().get_id(), [] (const rttr::variant& var) {	return writeJson(var.get_value<sim::Vector3>()); }},
		{ rttr::type::get<sim::Quaternion>().get_id(), [] (const rttr::variant& var) { return writeJson(var.get_value<sim::Quaternion>()); }},
		{ rttr::type::get<sim::LatLon>().get_id(), [] (const rttr::variant& var) { return writeJson(var.get_value<sim::LatLon>()); }},
		{ rttr::type::get<sim::LatLonAlt>().get_id(), [] (const rttr::variant& var) { return writeJson(var.get_value<sim::LatLonAlt>()); }}
	};

	rttr::variant unwrappedVar = unwrap(var);
	auto type = unwrappedVar.get_type();
	if (const auto& i = translators.find(type.get_id()); i != translators.end())
	{
		return (i->second)(unwrappedVar);
	}
	else if (type.is_sequential_container())
	{
		return sequentialContainerToJson(unwrappedVar);
	}
	else if (type.is_associative_container())
	{
		return associativeContainerToJson(unwrappedVar);
	}
	else
	{
		return writeReflectedObject(unwrappedVar);
	}

	return {};
}

static nlohmann::json sequentialContainerToJson(const rttr::variant& var)
{
	nlohmann::json json;

	rttr::variant_sequential_view view = var.create_sequential_view();
	for (const auto& value : view)
	{
		json.push_back(toJson(value));
	}
	return json;
}

static nlohmann::json associativeContainerToJson(const rttr::variant& var)
{
	nlohmann::json json;

	rttr::variant_associative_view view = var.create_associative_view();
	for (const auto& [key, value] : view)
	{
		json[key.to_string()] = toJson(value);
	}
	return json;
}

nlohmann::json writeReflectedObject(const rttr::instance& object)
{
	nlohmann::json json;

	const auto& type = object.get_derived_type();

	if (type.is_derived_from<ExplicitSerialization>())
	{
		if (ExplicitSerialization* serialization = object.try_convert<ExplicitSerialization>(); serialization)
		{
			json = serialization->toJson();
		}
		else
		{
			assert(!"Should not get here");
		}
	}
	else // use reflection based serialization
	{
		const auto& props = type.get_properties();
		for (const rttr::property& property : props)
		{
			if (isSerializable(property))
			{
				auto type = property.get_type();
				if (type.is_wrapper())
				{
					type = type.get_wrapped_type();
				}
				rttr::variant var = property.get_value(object);
				nlohmann::json valueJson = toJson(var);
				if (!valueJson.is_null())
				{
					json[property.get_name().to_string()] = valueJson;
				}
			}
		}
	}

	return json;
}

} // namespace skybolt::sim