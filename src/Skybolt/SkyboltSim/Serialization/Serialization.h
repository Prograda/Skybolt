/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltReflect/Reflection.h"

#include <nlohmann/json.hpp>

namespace skybolt::sim {

//! Reads object's properties from json. If object derives from ExplicitSerialization, the
//! ExplicitSerialization function will be used, otherwise fall back to readReflectedObjectProperties().
void readReflectedObject(refl::TypeRegistry& registry, refl::Instance& object, const nlohmann::json& json);

void readReflectedObjectProperties(refl::TypeRegistry& registry, refl::Instance& object, const nlohmann::json& json);

//! Writes object's properties to json. If object derives from ExplicitSerialization, the
//! ExplicitSerialization function will be used, otherwise fall back to writeReflectedObjectProperties().
nlohmann::json writeReflectedObject(refl::TypeRegistry& registry, const refl::Instance& object);

//! Writes object's properties to json
nlohmann::json writeReflectedObjectProperties(refl::TypeRegistry& registry, const refl::Instance& object);

//! If an object implements this class, the toJson/fromJson methods will be used to serialize the object,
//! instead of relying on reflection for automatic serialization. This is useful for implementing complex
//! serialization logic that can't be handled by rttr reflection.
struct ExplicitSerialization
{
public:
	virtual ~ExplicitSerialization() = default;

	virtual nlohmann::json toJson(refl::TypeRegistry& typeRegistry) const = 0;
	virtual void fromJson(refl::TypeRegistry& typeRegistry, const nlohmann::json& j) = 0;
};

} // namespace skybolt::sim
