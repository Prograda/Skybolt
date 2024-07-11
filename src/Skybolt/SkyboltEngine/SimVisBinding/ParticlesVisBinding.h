/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "EntityVisibilityFilterable.h"
#include "SimVisBinding.h"
#include <SkyboltVis/SkyboltVisFwd.h>

namespace skybolt {

class ParticlesVisBinding : public SimVisBinding
{
public:
	ParticlesVisBinding(const sim::ParticleSystemPtr& particleSystem, const vis::ParticlesPtr& particles);

public:
	// SimVisBinding interface
	void syncVis(const GeocentricToNedConverter& converter) override;


private:
	sim::ParticleSystemPtr mParticleSystem;
	vis::ParticlesPtr mParticles;
	osg::ref_ptr<osg::Vec3Array> mParticlePositions;
};

} // namespace skybolt