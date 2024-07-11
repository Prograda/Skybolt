/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

namespace skybolt {
namespace sim {

class CollisionGroupMasks
{
public:
	static const int terrain = 1;
	static const int simBody = 1 << 1;
};

} // namespace sim
} // namespace skybolt
