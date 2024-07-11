/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace vis {

template <typename T>
struct Rect
{
	Rect(T x, T y, T width, T height) :
		x(x), y(y), width(width), height(height) {}

	Rect() {}

 T x;
 T y;
 T width;
 T height;
};

typedef Rect<int> RectI;
typedef Rect<float> RectF;

} // namespace vis
} // namespace skybolt
