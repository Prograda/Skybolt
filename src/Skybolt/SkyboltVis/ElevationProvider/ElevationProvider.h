/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace vis {

class ElevationProvider
{
public:
	virtual ~ElevationProvider() {}
	virtual float get(float x, float y) const = 0; //!< Returns Z coordinate of terrain at X,Y point. -ve is up.
};

} // namespace vis
} // namespace skybolt
