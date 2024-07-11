/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GpuForestTile.h"
#include "BillboardForest.h"

#include "SkyboltVis/GeoImageHelpers.h"
#include "SkyboltVis/LlaToNedConverter.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/VisibilityCategory.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <osg/Geode>
#include <atomic>
#include <deque>
#include <mutex>
#include <thread>

using namespace skybolt;

namespace skybolt {
namespace vis {

GpuForestTile::GpuForestTile(const osg::ref_ptr<osg::Texture2D>& heightMap, const osg::ref_ptr<osg::Texture2D>& attributeMap, const std::shared_ptr<BillboardForest>& forest, const osg::Vec2f& tileWorldSizeMeters)
{
	mTransform->setNodeMask(vis::VisibilityCategory::defaultCategories | vis::VisibilityCategory::shadowCaster);

	osg::StateSet* stateSet = mTransform->getOrCreateStateSet();
	stateSet->addUniform(new osg::Uniform("tileWorldSize", tileWorldSizeMeters));

	int unit = 2;
	{
		stateSet->setTextureAttributeAndModes(unit, heightMap);
		stateSet->addUniform(createUniformSampler2d("heightSampler", unit++));
		// TODO: set clamping mode. Height points should be on edges of terrain

		stateSet->setTextureAttributeAndModes(unit, attributeMap);
		stateSet->addUniform(createUniformSampler2d("attributeSampler", unit++));
	}

	stateSet->setDefine("GPU_PLACEMENT");

	mTransform->addChild(forest->_getNode());

	mModelMatrixUniform = new osg::Uniform("modelMatrix", osg::Matrixf());
	stateSet->addUniform(mModelMatrixUniform);
}

void GpuForestTile::updatePreRender(const CameraRenderContext& context)
{
	osg::Matrix modelMatrix = mTransform->getWorldMatrices().front();
	mModelMatrixUniform->set(modelMatrix);
}

} // namespace vis
} // namespace skybolt
