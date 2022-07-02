/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/RenderOperation/RenderOperation.h"
#include "SkyboltVis/TextureGenerator/GpuTextureGenerator.h"

namespace skybolt {
namespace vis {

class WaterMaterialRenderOperation : public RenderOperation
{
public:
	WaterMaterialRenderOperation(const ScenePtr& scene, const ShaderPrograms& programs);
	~WaterMaterialRenderOperation() override = default;

	void updatePreRender(const RenderContext& renderContext) override;

	std::vector<osg::ref_ptr<osg::Texture>> getOutputTextures() const override;

private:
	ScenePtr mScene;
	osg::ref_ptr<GpuTextureGenerator> mGenerator;
};

} // namespace vis
} // namespace skybolt