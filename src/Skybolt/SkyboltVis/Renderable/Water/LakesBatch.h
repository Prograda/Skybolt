/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/VisObject.h"
#include <osg/MatrixTransform>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct Lake
{
	std::vector<osg::Vec3f> points;
};

typedef std::vector<Lake> Lakes;

struct LakesConfig
{
	osg::ref_ptr<osg::Program> program;
	osg::ref_ptr<osg::StateSet> waterStateSet;
};

class LakesBatch : public VisObject
{
public:
	struct Uniforms
	{
		osg::Uniform* modelMatrix;
	};

	LakesBatch(const Lakes& lakes, const LakesConfig& config);

	//! Sinks heightmap under lakes
	void sinkHeightmap(osg::Image& image);

	osg::Node* _getNode() const { return mTransform; }

protected:
	void updatePreRender(const RenderContext& context) override;

private:
	osg::MatrixTransform* mTransform;
	Uniforms mUniforms;
};

} // namespace vis
} // namespace skybolt
