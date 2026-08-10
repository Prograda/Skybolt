/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RenderOperationUtil.h"

#include "SkyboltVisOsg/RenderContext.h"
#include "SkyboltVisOsg/RenderOperation/RenderOperationOrder.h"
#include "SkyboltVisOsg/RenderOperation/RenderOperationSequence.h"
#include "SkyboltVisOsg/RenderOperation/RenderOperationVisualizer.h"
#include "SkyboltVisOsg/Shader/ShaderProgramRegistry.h"

#include <assert.h>

namespace skybolt {
namespace vis {

osg::ref_ptr<RenderOperation> createRenderOperationVisualization(const osg::ref_ptr<RenderOperation>& rop, const ShaderPrograms& registry)
{
	return new RenderOperationVisualizer(rop, registry.getRequiredProgram("hudGeometry"), registry.getRequiredProgram("hudTexture3d"));
}

class RenderOperationFunction : public RenderOperation
{
public:
	RenderOperationFunction(const std::function<void(const RenderContext&)>& func) :
		mFunc(func)
	{
		assert(mFunc);
	}

	void updatePreRender(const RenderContext& renderContext) override
	{
		mFunc(renderContext);
	}

private:
	std::function<void(const RenderContext&)> mFunc;
};

osg::ref_ptr<RenderOperation> createRenderOperationFunction(std::function<void(const RenderContext&)> func)
{
	return new RenderOperationFunction(std::move(func));
}

} // namespace vis
} // namespace skybolt
