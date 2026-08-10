#pragma once

#include "ElevationBounds.h"
#include "ElevationRerange.h"
#include "SkyboltVis/Image/Image.h"

namespace skybolt::vis {

struct ElevationImageMetadata
{
	ElevationBounds elevationBounds = emptyElevationBounds();
	ElevationRerange rerange = { 1, 0 };
};

std::optional<ElevationImageMetadata> getElevationImageMetadata(const Image& image);
ElevationImageMetadata getElevationImageMetadataRequired(const Image& image); //!< throws exception if not found

void setElevationImageMetadata(Image& image, const ElevationImageMetadata& metadata);

} // namespace skybolt::vis