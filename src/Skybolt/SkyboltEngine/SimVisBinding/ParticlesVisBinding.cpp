/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ParticlesVisBinding.h"
#include "GeocentricToNedConverter.h"
#include <SkyboltSim/Particles/ParticleSystem.h>
#include <SkyboltVis/Renderable/Particles.h>
#include <assert.h>

namespace skybolt {

constexpr double forceScaleMetersPerNewton = 0.0001;

ParticlesVisBinding::ParticlesVisBinding(const sim::ParticleSystemPtr& particleSystem, const vis::ParticlesPtr& particles) :
	mParticleSystem(particleSystem),
	mParticles(particles),
	mParticlePositions(new osg::Vec3Array())
{
	assert(mParticleSystem);
	assert(mParticles);
}

void ParticlesVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	auto simParticles = mParticleSystem->getParticles();
	mParticlePositions->resize(simParticles.size());

	int i = 0;
	for (const auto& simParticle : simParticles)
	{
		(*mParticlePositions)[i] = converter.convertPosition(simParticle.position);
		++i;
	}

	mParticles->setParticles(simParticles, mParticlePositions);
}

} // namespace skybolt
