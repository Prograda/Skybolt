/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nlohmann/json.hpp>

class JsonScenarioSerializable
{
public:
	virtual ~JsonScenarioSerializable() = default;

	virtual void resetScenario() {};
	virtual void readScenario(const nlohmann::json& json) {};
	virtual void writeScenario(nlohmann::json& json) const {};
};
