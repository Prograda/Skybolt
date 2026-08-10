#pragma once

#include "SkyboltVis/Image/Image.h"

#include <glm/glm.hpp>
#include <optional>

namespace skybolt {
namespace vis {

//! Stores { scale, offset } for converting from image color value to elevation, as follows:
//! elevationInMeters = colorValue * scale + offset
using ElevationRerange = glm::vec2;

inline ElevationRerange rerangeElevationFromUInt16WithElevationBounds(float minElevation, float maxElevation)
{
	float scale = (maxElevation - minElevation) / 65535;
	return { scale, minElevation };
}

inline const ElevationRerange& getDefaultEarthRerange()
{
	static ElevationRerange r = rerangeElevationFromUInt16WithElevationBounds(-500, 8850);
	return r;
}

int getColorValueForElevation(const ElevationRerange& rerange, float elevation);
float getElevationForColorValue(const ElevationRerange& rerange, int value);

} // namespace vis
} // namespace skybolt