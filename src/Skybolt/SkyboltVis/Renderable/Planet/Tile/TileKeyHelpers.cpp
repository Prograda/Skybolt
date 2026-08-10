/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileKeyHelpers.h"

namespace skybolt {
namespace vis {

void getTileTransformInParentSpace(const QuadTreeTileKey& key, int parentLod, glm::vec2& scale, glm::vec2& offset, bool flipV)
{
	int reductions = key.level - parentLod;
	assert(reductions >= 0);

	int scaleInt = 1 << reductions;

	glm::ivec2 reducedIndex(key.x / scaleInt, key.y / scaleInt);
	glm::ivec2 v = reducedIndex * scaleInt;

	float rcpScale = 1.0f / (float)scaleInt;
	scale = glm::vec2(rcpScale, rcpScale);

	if (flipV)
	{
		offset = glm::vec2(key.x - v.x, (scaleInt - 1) - (key.y - v.y)) * rcpScale;
	}
	else
	{
		offset = glm::vec2(key.x - v.x, key.y - v.y) * rcpScale;
	}
}

} // namespace vis
} // namespace skybolt
