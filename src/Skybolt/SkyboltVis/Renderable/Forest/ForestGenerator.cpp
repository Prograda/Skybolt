/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ForestGenerator.h"
#include <SkyboltCommon/Math/MathUtility.h>

std::vector<float> treeTypeHeights = {45, 45, 45};
const float minTreeZ = -5.f; //!< Trees will not be generated below this world Z coordinate

namespace skybolt {
namespace vis {

ForestGenerator::ForestGenerator() :
	random(0)
{
}

std::vector<BillboardForest::Tree> ForestGenerator::generate(const ElevationProvider& elevation, const AttributeImage& image, const Box2f& worldBounds, const Box2f& imageBounds)
{
	std::vector<BillboardForest::Tree> result;

	osg::Vec2f cellSize = worldBounds.maximum - worldBounds.minimum;
	cellSize.x() /= imageBounds.maximum.y() - imageBounds.minimum.y();
	cellSize.y() /= imageBounds.maximum.x() - imageBounds.minimum.x();

	osg::Vec2f offset(worldBounds.minimum.x() - cellSize.x() * imageBounds.minimum.y(),
					  worldBounds.minimum.y() - cellSize.y() * imageBounds.minimum.x());

	int xStart = std::max(0, (int)imageBounds.minimum.x());
	int yStart = std::max(0, (int)imageBounds.minimum.y());
	int xEnd = std::min(image.width, (int)ceil(imageBounds.maximum.x()));
	int yEnd = std::min(image.height, (int)ceil(imageBounds.maximum.y()));

	for (int y = yStart; y < yEnd; ++y)
	{
		for (int x = xStart; x < xEnd; ++x)
		{
			int attribute = image.data[4 * (x + y * image.width) + 3];
			Attributes::const_iterator it = image.attributes.find(attribute);
			if (it != image.attributes.end())
			{
				const Attribute& attr = it->second;

				float xMin = std::max(float(y), imageBounds.minimum.y()) * cellSize.x() + offset.x();
				float yMin = std::max(float(x), imageBounds.minimum.x()) * cellSize.y() + offset.y();
				float xMax = std::min(float(y + 1), imageBounds.maximum.y()) * cellSize.x() + offset.x();
				float yMax = std::min(float(x + 1), imageBounds.maximum.x()) * cellSize.y() + offset.y();

				double area = (xMax - xMin) * (yMax - yMin);

				float treeCountF = (double)attr.density * area;

				// account for fractional trees
				int treeCount = treeCountF;
				if (treeCountF - (float)treeCount > random.unitRand())
					++treeCount;

				// add trees
				for (int t = 0; t < treeCount; ++t)
				{
					BillboardForest::Tree tree;
					tree.position.x() = skybolt::math::lerp(xMin, xMax, random.unitRand()); // TODO: use a more uniformly spaced random number sequence
					tree.position.y() = skybolt::math::lerp(yMin, yMax, random.unitRand());
					tree.position.z() = elevation.get(tree.position.x(), tree.position.y());

					if (tree.position.z() < minTreeZ)
					{
						tree.type = std::min((int)treeTypeHeights.size()-1, int(random.unitRand() * treeTypeHeights.size()));
						tree.height = treeTypeHeights[tree.type] * skybolt::math::lerp(0.75f, 1.25f, random.unitRand());
						tree.yaw = 2.f * osg::PI * random.unitRand();
						result.push_back(tree);
					}
				}
			}
		}
	}

	return result;
}

} // namespace vis
} // namespace skybolt
