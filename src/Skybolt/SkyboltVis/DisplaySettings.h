#pragma once

#include <nlohmann/json.hpp>

namespace skybolt {
namespace vis {

//! Default constructed with reasonable defaults
struct DisplaySettings
{
	int multiSampleCount = 4; //!< Number of samples for MSAA
	int texturePoolSizeBytes =  512 * 1024 * 1024;
	bool vsync = true;
};

} // namespace vis
} // namespace skybolt