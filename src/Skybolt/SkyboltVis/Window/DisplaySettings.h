#pragma once

#include <nlohmann/json.hpp>

namespace skybolt {
namespace vis {

//! Default constructed with reasonable defaults
struct DisplaySettings
{
	int multiSampleCount = 4; //!< Number of samples for MSAA
};

} // namespace vis
} // namespace skybolt