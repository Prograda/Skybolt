/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "DefaultRootNode.h"
#include "OsgBox2.h"

namespace skybolt {
namespace vis {

struct Road
{
	std::vector<osg::Vec3f> points;
	int laneCount;
	float width; //!< total width of all lanes
};

typedef std::vector<Road> Roads;

struct PolyRegion
{
	std::vector<osg::Vec3f> points;
};

typedef std::vector<PolyRegion> PolyRegions;

class RoadsBatch : public DefaultRootNode
{
public:
	struct Uniforms
	{
		osg::Uniform* modelMatrix;
	};

	RoadsBatch(const Roads& roads, const osg::ref_ptr<osg::Program>& program);
	RoadsBatch(const PolyRegions& regions, const osg::ref_ptr<osg::Program>& program);

	//! Replaces attribute along roads
	static void replaceAttribute(const Roads& roads, osg::Image& image, const Box2f& imageWorldBounds, int attributeToReplace, int attributeToReplaceWith);

protected:
	void updatePreRender(const RenderContext& context);

private:
	Uniforms mUniforms;
};

} // namespace vis
} // namespace skybolt
