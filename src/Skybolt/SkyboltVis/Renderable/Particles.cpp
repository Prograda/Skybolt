/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Particles.h"
#include "SkyboltVis/OsgGeometryHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include <SkyboltSim/Particles/ParticleSystem.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <glm/glm.hpp>

using namespace skybolt;
using namespace skybolt::vis;

Particles::Particles(const osg::ref_ptr<osg::Program>& program, const osg::ref_ptr<osg::Texture2D>& albedoTexture) :
	mGeometry(new osg::Geometry()),
	mDrawArrays(new osg::DrawArrays()),
	mParticleVertices(new osg::Vec3Array()),
	mParticleUvs(new osg::Vec3Array())
{
	mGeometry->addPrimitiveSet(mDrawArrays);
	configureDrawable(*mGeometry);

	auto stateSet = mGeometry->getOrCreateStateSet();
	stateSet->setAttribute(program);
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	makeStateSetTransparent(*stateSet, TransparencyMode::Classic);

	int unit = 0;
	stateSet->setTextureAttributeAndModes(unit, albedoTexture);
	stateSet->addUniform(createUniformSampler2d("albedoSampler", unit++));

	mTransform->addChild(mGeometry);
}

static float randomFast(float n) { return glm::fract(sin(n) * 43758.5453123); }

void Particles::setParticles(const std::vector<sim::Particle>& particles, const osg::ref_ptr<osg::Vec3Array>& visParticlePositions)
{
	assert(visParticlePositions->size() == particles.size());
	mParticleVertices->reserve(particles.size() * 4);
	mParticleVertices->clear();
	mParticleUvs->reserve(particles.size() * 4);
	mParticleUvs->clear();

	osg::BoundingBox bounds;
	int i = 0;
	for (const auto& pos : *visParticlePositions)
	{
		const auto& particle = particles[i];
		float radius = particle.radius;
		osg::Vec3f corner(radius, radius, radius);
		osg::BoundingBox box(pos - corner, pos + corner);
		bounds.expandBy(box);

		const float rotation = randomFast(particle.guid % 100000) * math::twoPiF();

		for (int j = 0; j < 4; ++j)
		{
			mParticleVertices->push_back(pos);
			mParticleUvs->push_back(osg::Vec3(radius, particle.alpha, rotation));
		}

		++i;
	}
	mGeometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(bounds));

	mGeometry->setVertexArray(mParticleVertices);
	mGeometry->setTexCoordArray(0, mParticleUvs);
	mDrawArrays->set(osg::PrimitiveSet::QUADS, 0, mParticleVertices->size());
}
