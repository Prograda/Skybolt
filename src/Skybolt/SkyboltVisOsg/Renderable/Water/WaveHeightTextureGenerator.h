/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltCommon/Registry.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/TypedItemContainer.h>
#include <SkyboltSim/OceanSurfaceSampler.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltVis/VisFactory.h>
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

	virtual float getWindVelocityHeading() const = 0;
	virtual void setWindVelocityHeading(float heading) = 0;

	virtual int getTextureCount() const = 0; //! Texture count must be constant for the lifetime of the object
	virtual osg::ref_ptr<osg::Texture2D> getTexture(int index) const = 0;
	virtual float getTextureWorldSize(int index) const = 0;

	//! @returns an interface for sampling the ocean surface eometry. Result is never null.
	virtual skybolt::sim::OceanSurfaceSamplerPtr getSurfaceSampler() const
	{
		static auto sampler = std::make_shared<sim::PlanarOceanSurfaceSampler>();
		return sampler;
	}
};

class WaveHeightTextureGeneratorFactory : public VisFactoryT<WaveHeightTextureGeneratorFactory>
{
public:
	~WaveHeightTextureGeneratorFactory() override = default;
	virtual std::unique_ptr<WaveHeightTextureGenerator> create() const = 0;
};

} // namespace vis

template<>
inline std::vector<std::type_index> getExposedTypes<vis::WaveHeightTextureGeneratorFactory>(const vis::WaveHeightTextureGeneratorFactory& type)
{
	return { typeid(type), typeid(vis::WaveHeightTextureGeneratorFactory) };
} 

} // namespace skybolt
