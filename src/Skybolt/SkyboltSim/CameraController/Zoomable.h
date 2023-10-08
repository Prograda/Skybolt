/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltReflection/Reflection.h>

namespace skybolt {
namespace sim {

class Zoomable
{
public:
	virtual ~Zoomable() = default;
	virtual double getZoom() const { return mZoom; }
	virtual void setZoom(double zoom) { mZoom = zoom; }

protected:
	double mZoom = 0;
};

SKYBOLT_REFLECT_BEGIN(Zoomable)
{
	registry.type<Zoomable>("Zoomable")
		.property("zoom", &Zoomable::getZoom, &Zoomable::setZoom);
}
SKYBOLT_REFLECT_END

} // namespace sim
} // namespace skybolt