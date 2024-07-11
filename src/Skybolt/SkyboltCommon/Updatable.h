/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

namespace skybolt {

class Updatable
{
public:
	virtual ~Updatable() {}
	virtual void update() = 0;
};

} // namespace skybolt