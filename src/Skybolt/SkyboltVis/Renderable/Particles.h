/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <osg/Geometry>
#include <osg/Program>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

class Particles : public DefaultRootNode
{
public:
	Particles(const osg::ref_ptr<osg::Program>& program, const osg::ref_ptr<osg::Texture2D>& albedoTexture);
	~Particles() override = default;

	void setParticles(const std::vector<sim::Particle>& particles, const osg::ref_ptr<osg::Vec3Array>& visParticlePositions);

private:
	osg::ref_ptr<osg::Geometry> mGeometry;
	osg::ref_ptr<osg::DrawArrays> mDrawArrays;
	osg::ref_ptr<osg::Vec3Array> mParticleVertices;
	osg::ref_ptr<osg::Vec4Array> mParticleUvs;
};

} // namespace vis
} // namespace skybolt
