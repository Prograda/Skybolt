/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>

namespace skybolt {

class UpdateLoop
{
public:
	UpdateLoop(float minFrameDuration);

	typedef std::function<bool(float dt)> Updatable;
	void exec(Updatable updatable);

private:
	float mMinFrameDuration;
};

} // namespace skybolt