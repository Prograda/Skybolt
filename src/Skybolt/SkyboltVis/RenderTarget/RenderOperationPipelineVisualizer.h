/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "RenderOperation.h"

namespace osg {
class Camera;
class Program;
}

namespace skybolt {
namespace vis {

//! Represents a piece of work to be performed during render
class RenderOperationPipelineVisualizer : public RenderOperation
{
public:
	RenderOperationPipelineVisualizer(RenderOperationPipeline* pipeline, const osg::ref_ptr<osg::Program>& program);
	void updatePreRender() override;

private:
	RenderOperationPipeline* mPipeline;
	osg::ref_ptr<osg::Program> mProgram;
	osg::ref_ptr<osg::Camera> mCamera;

	std::vector<osg::ref_ptr<osg::Texture>> mPrevTextures;
	std::vector<vis::VisObjectPtr> mScreenQuads;
};

} // namespace vis
} // namespace skybolt
