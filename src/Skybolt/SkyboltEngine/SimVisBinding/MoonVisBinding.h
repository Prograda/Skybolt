/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "CelestialObjectVisBinding.h"

namespace skybolt {

class MoonVisBinding : public CelestialObjectVisBinding
{
public:
	MoonVisBinding(JulianDateProvider dateProvider, osg::Uniform* moonPhaseUniform, const vis::RootNodePtr& visObject);
	void syncVis(const GeocentricToNedConverter& converter) override;

private:
	osg::Uniform* mMoonPhaseUniform;
};

} // namespace skybolt