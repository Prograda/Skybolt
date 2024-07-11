/* Copyright Matthew Reid
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

class RenderOperationVisualizer : public RenderOperation
{
public:
	RenderOperationVisualizer(const osg::ref_ptr<RenderOperation>& renderOperation, const osg::ref_ptr<osg::Program>& program);
	void updatePreRender(const RenderContext& renderContext) override;

private:
	osg::ref_ptr<RenderOperation> mRenderOperation;
	osg::ref_ptr<osg::Program> mProgram;
	osg::ref_ptr<osg::Camera> mCamera;

	std::vector<osg::ref_ptr<osg::Texture>> mPrevTextures;
	std::vector<vis::VisObjectPtr> mScreenQuads;
};

} // namespace vis
} // namespace skybolt
