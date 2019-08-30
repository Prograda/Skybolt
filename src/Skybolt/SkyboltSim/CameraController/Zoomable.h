/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace sim {

	class Zoomable
	{
	public:
		virtual ~Zoomable() = default;
		virtual float getZoom() const { return mZoom; }
		virtual void setZoom(float zoom) { mZoom = zoom; }

	protected:
		float mZoom = 0;
	};

} // namespace sim
} // namespace skybolt