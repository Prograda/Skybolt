#pragma once

#include <osg/Vec2>
#include <optional>

namespace osg {
class Image;
}

namespace skybolt {
namespace vis {

//! [minimum, maximum] elevation bounds in meters
using HeightMapElevationBounds = osg::Vec2f;

inline HeightMapElevationBounds emptyHeightMapElevationBounds()
{
	static HeightMapElevationBounds b(std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
	return b;
}

inline void expand(HeightMapElevationBounds& b, float p)
{
	b.x() = osg::minimum(b.x(), p);
	b.y() = osg::maximum(b.y(), p);
}

inline void expand(HeightMapElevationBounds& b, const HeightMapElevationBounds& other)
{
	b.x() = osg::minimum(b.x(), other.x());
	b.y() = osg::maximum(b.y(), other.y());
}

// Helper functions for tagging an image with bounds meta-data
std::optional<HeightMapElevationBounds> getHeightMapElevationBounds(const osg::Image& image);
HeightMapElevationBounds getRequiredHeightMapElevationBounds(const osg::Image& image);
void setHeightMapElevationBounds(osg::Image& image, const HeightMapElevationBounds& bounds);

} // namespace vis
} // namespace skybolt