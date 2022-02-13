#pragma once

namespace skybolt {
namespace vis {

inline int getHeightmapSeaLevelValueInt() { return 32767; }
inline float getHeightmapSeaLevelValueFloat() { return 32767.f; }
inline float getHeightmapMinAltitude() { return -32767.f; } //!< Return value in meters

} // namespace vis
} // namespace skybolt