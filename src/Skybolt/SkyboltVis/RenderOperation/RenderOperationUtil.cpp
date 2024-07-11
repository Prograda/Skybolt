/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RenderOperationUtil.h"

#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderOperation/RenderOperationOrder.h"
#include "SkyboltVis/RenderOperation/RenderOperationSequence.h"
#include "SkyboltVis/RenderOperation/RenderOperationVisualizer.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"

#include <assert.h>

namespace skybolt {
namespace vis {

osg::ref_ptr<RenderOperation> createRenderOperationVisualization(const osg::ref_ptr<RenderOperation>& rop, const ShaderPrograms& registry)
{
	return new RenderOperationVisualizer(rop, registry.getRequiredProgram("hudGeometry"));
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
