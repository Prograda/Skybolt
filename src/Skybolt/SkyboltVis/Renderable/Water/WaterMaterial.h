/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Renderable/Water/WaterStateSet.h"

#include <osg/Group>

namespace skybolt {
namespace vis {

class WaveHeightTextureGeneratorFactory;

struct WaterMaterialConfig
{
	const WaveHeightTextureGeneratorFactory* factory;
	const ShaderPrograms* programs;
};

class WaterMaterial : public osg::Group
{
public:
	WaterMaterial(const WaterMaterialConfig& config);

	void setWaveHeight(float height);
	float getWaveHeight() const;

	void setWakes(const std::vector<Wake>& wakes);

	void update(double timeSeconds);

	osg::ref_ptr<osg::Texture2D> getHeightTexture(int cascadeIndex) const;
	osg::ref_ptr<osg::Texture2D> getNormalTexture(int cascadeIndex) const;
	osg::ref_ptr<osg::Texture2D> getFoamMaskTexture(int cascadeIndex) const;
	int getCascadeCount() { return WaterStateSetConfig::waveTextureCount; }

private:
	std::unique_ptr<class CascadedWaveHeightTextureGenerator> mWaveHeightTextureGenerator;
	std::vector<osg::ref_ptr<GpuTextureGenerator>> mWaterSurfaceGpuTextureGenerators; //!< stored in execution order
	osg::ref_ptr<class WaveFoamMaskGenerator> mWaveFoamMaskGenerator[WaterStateSetConfig::waveTextureCount];
};

} // namespace vis
} // namespace skybolt
