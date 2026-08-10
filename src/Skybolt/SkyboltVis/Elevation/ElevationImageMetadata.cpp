#pragma once

#include "ElevationImageMetadata.h"
#include <SkyboltCommon/Exception.h>

namespace skybolt::vis {

std::string elevationImageMetadataKey = "elevation";

std::optional<ElevationImageMetadata> getElevationImageMetadata(const Image& image)
{
	if (auto value = image.getMetadata(elevationImageMetadataKey); value)
	{
		if (std::holds_alternative<glm::vec4>(*value))
		{
			auto v = std::get<glm::vec4>(*value);
			ElevationImageMetadata metadata;
			metadata.elevationBounds = { v.x, v.y };
			metadata.rerange = { v.z, v.w };
			return metadata;
		}
	}
	return std::nullopt;
}

ElevationImageMetadata getElevationImageMetadataRequired(const Image& image)
{
	if (auto result = getElevationImageMetadata(image); result)
	{
		return *result;
	}
	throw Exception("ElevationImageMetadata not found");
}

void setElevationImageMetadata(Image& image, const ElevationImageMetadata& metadata)
{
	glm::vec4 v;
	v.x = metadata.elevationBounds.x;
	v.y = metadata.elevationBounds.y;
	v.z = metadata.rerange.x;
	v.w = metadata.rerange.y;

	image.addMetadata(elevationImageMetadataKey, v);
}

} // namespace skybolt::vis