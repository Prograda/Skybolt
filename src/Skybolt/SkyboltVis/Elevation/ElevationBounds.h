#pragma once

#include <glm/glm.hpp>
#include <optional>

namespace skybolt {
namespace vis {

//! [minimum, maximum] elevation bounds in meters
using ElevationBounds = glm::vec2;

inline ElevationBounds emptyElevationBounds()
{
	static ElevationBounds b(std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
	return b;
}

inline void expand(ElevationBounds& b, float p)
{
	b.x = glm::min(b.x, p);
	b.y = glm::max(b.y, p);
}

inline void expand(ElevationBounds& b, const ElevationBounds& other)
{
	b.x = glm::min(b.x, other.x);
	b.y = glm::max(b.y, other.y);
}

} // namespace vis
} // namespace skybolt