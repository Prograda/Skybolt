/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimMath.h"
#include "SkyboltSim/Spatial/LatLon.h"
#include "SkyboltSim/Spatial/LatLonAlt.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <nlohmann/json.hpp>

namespace skybolt {
namespace sim {

inline Vector3 readVector3(const nlohmann::json& j)
{
	return Vector3(j[0].get<double>(), j[1].get<double>(), j[2].get<double>());
}

inline nlohmann::json writeJson(const Vector3& v)
{
	return {v[0], v[1], v[2]};
}

inline Vector3 readOptionalVector3(const nlohmann::json& j, const std::string& name)
{
	auto i = j.find(name);
	if (i != j.end())
	{
		return readVector3(*i);
	}
	return math::dvec3Zero();
}

inline Quaternion readQuaternion(const nlohmann::json& j)
{
	return glm::angleAxis(j["angleDeg"].get<double>() * skybolt::math::degToRadD(), readVector3(j["axis"]));
}

inline nlohmann::json writeJson(const Quaternion& v)
{
	nlohmann::json j;
	
	j["angleDeg"] = glm::angle(v) * skybolt::math::radToDegD();
	j["axis"] = writeJson(glm::axis(v));
	return j;
}

inline Quaternion readOptionalQuaternion(const nlohmann::json& j, const std::string& name)
{
	auto i = j.find(name);
	if (i != j.end())
	{
		return readQuaternion(*i);
	}
	return math::dquatIdentity();
}

inline LatLon readLatLon(const nlohmann::json& j)
{
	return LatLon(j[0].get<double>() * skybolt::math::degToRadD(), j[1].get<double>() * skybolt::math::degToRadD());
}

inline nlohmann::json writeJson(const LatLon& v)
{
	return {v.lat * skybolt::math::radToDegD(), v.lon * skybolt::math::radToDegD()};
}

inline LatLonAlt readLatLonAlt(const nlohmann::json& j)
{
	return LatLonAlt(j[0].get<double>() * skybolt::math::degToRadD(), j[1].get<double>() * skybolt::math::degToRadD(), j[2].get<double>());
}

inline nlohmann::json writeJson(const LatLonAlt& v)
{
	return {v.lat * skybolt::math::radToDegD(), v.lon * skybolt::math::radToDegD(), v.alt};
}

inline void writeIfNotEmpty(nlohmann::json& object, const std::string& key, const nlohmann::json& v)
{
	if (!v.empty())
	{
		object[key] = v;
	}
}

} // namespace sim
} // namespace skybolt