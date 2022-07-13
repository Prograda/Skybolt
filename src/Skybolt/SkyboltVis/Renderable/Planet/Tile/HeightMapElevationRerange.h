#pragma once

#include <osg/Vec2>
#include <optional>

namespace osg {
class Image;
}

namespace skybolt {
namespace vis {

//! Stores { scale, offset } for converting from image color value to elevation, as follows:
//! elevationInMeters = colorValue * scale + offset
using HeightMapElevationRerange = osg::Vec2f;

inline HeightMapElevationRerange rerangeElevationFromUInt16WithElevationBounds(float minElevation, float maxElevation)
{
	float scale = (maxElevation - minElevation) / 65535;
	return { scale, minElevation };
}

inline const HeightMapElevationRerange& getDefaultEarthRerange()
{
	static HeightMapElevationRerange r = rerangeElevationFromUInt16WithElevationBounds(-500, 8850);
	return r;
}

int getColorValueForElevation(const HeightMapElevationRerange& rerange, float elevation);
float getElevationForColorValue(const HeightMapElevationRerange& rerange, int value);

std::optional<HeightMapElevationRerange> getHeightMapElevationRerange(const osg::Image& image);
HeightMapElevationRerange getRequiredHeightMapElevationRerange(const osg::Image& image);
void setHeightMapElevationRerange(osg::Image& image, const HeightMapElevationRerange& rerange);

} // namespace vis
} // namespace skybolt