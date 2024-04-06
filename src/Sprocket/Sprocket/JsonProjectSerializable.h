/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nlohmann/json.hpp>

class JsonProjectSerializable
{
public:
	virtual ~JsonProjectSerializable() = default;

	virtual void resetProject() {};
	virtual void readProject(const nlohmann::json& json) {};
	virtual void writeProject(nlohmann::json& json) const {};
};
