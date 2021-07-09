/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct Wake
{
	std::vector<osg::Vec3f> points; //!< x, y, width
};

struct WaterStateSetConfig
{
	static const int waveTextureCount = 1;
	osg::ref_ptr<osg::Texture2D> waveHeightTexture[waveTextureCount];
	osg::ref_ptr<osg::Texture2D> waveNormalTexture[waveTextureCount];
	osg::ref_ptr<osg::Texture2D> waveFoamMaskTexture[waveTextureCount];

	float waveHeightMapWorldSizes[waveTextureCount];
};

//! StateSet containing textures and uniforms for a generic water shader.
//! Does not contain the shader program.
class WaterStateSet : public osg::StateSet
{
public:
	WaterStateSet(const WaterStateSetConfig& config);

	void setWakes(const std::vector<Wake>& wakes);
	void setFoamTexture(int index, osg::ref_ptr<osg::Texture2D> texture);

private:
	osg::ref_ptr<osg::Image> mWakeHashMapImage; //!< Maps a hashed wake spatial coordinate to {wake params first index, wake params last index + 1}
	osg::ref_ptr<osg::Image> mWakeParamsImage;
	int mFirstFoamMaskTextureIndex;
};

} // namespace vis
} // namespace skybolt
