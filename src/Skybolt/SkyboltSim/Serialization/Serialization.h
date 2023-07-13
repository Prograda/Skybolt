/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Reflection.h"

#include <nlohmann/json.hpp>

namespace skybolt::sim {

void readReflectedObject(rttr::instance& object, const nlohmann::json& json);

nlohmann::json writeReflectedObject(const rttr::instance& object);


//! If the rttr::instance object implements this class, the toJson/fromJson methods will be
//! used to serialize the object, instead of relying on rttr reflection for automatic serialization.
//! This is useful for implementing complex serialization logic that can't be handled by rttr reflection.
struct ExplicitSerialization
{
	SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION();
public:
	virtual ~ExplicitSerialization() = default;

	virtual nlohmann::json toJson() const = 0;
	virtual void fromJson(const nlohmann::json& j) = 0;
};

} // namespace skybolt::sim
