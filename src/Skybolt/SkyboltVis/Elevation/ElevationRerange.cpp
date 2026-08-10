#include "ElevationRerange.h"

namespace skybolt {
namespace vis {

int getColorValueForElevation(const ElevationRerange& rerange, float elevation)
{
	return int(std::round((elevation - rerange.y) / rerange.x));
}

float getElevationForColorValue(const ElevationRerange& rerange, int value)
{
	return value * rerange.x + rerange.y;
}

} // namespace vis
} // namespace skybolt