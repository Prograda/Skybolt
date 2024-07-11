/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/VisFactory.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

class WaveHeightTextureGenerator
{
public:
	virtual ~WaveHeightTextureGenerator() {}

	//! @return true if the texture was generated or re-generated, or false if the previously generated texture should be used.
	virtual bool generate(double time) = 0;

	virtual float getWaveHeight() const = 0;
	virtual void setWaveHeight(float height) = 0;

	virtual osg::ref_ptr<osg::Texture2D> getTexture() const = 0;
};

class WaveHeightTextureGeneratorFactory : public VisFactory
{
public:
	virtual std::unique_ptr<WaveHeightTextureGenerator> create(float textureWorldSize, const glm::vec2& normalizedFrequencyRange) const = 0;
};

} // namespace vis
} // namespace skybolt
