/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <btBulletDynamicsCommon.h>
#include <string>

namespace skybolt {
namespace sim {

class ShapeSerializer
{
public:
	static void save(const std::string& filepath, const btCollisionShape& shape);
	static btCollisionShape* load(const std::string& filepath);
};

} // namespace skybolt
} // namespace sim
