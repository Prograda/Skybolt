/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include "Helpers/CheckingHelpers.h"
#include <SkyboltVis/Renderable/Planet/Tile/NormalMapHelpers.h>

#include <osg/Image>
#include <osg/Texture>

using namespace skybolt;
using namespace skybolt::vis;

static osg::ref_ptr<osg::Image> create4x4TestHeightImage()
{
	osg::Image* image = new osg::Image();
	image->allocateImage(4, 4, 1, GL_LUMINANCE, GL_UNSIGNED_SHORT);
	image->setInternalTextureFormat(GL_R16);
	return image;
}

static osg::Vec3f readNormal(const osg::Image& image, int x, int y)
{
	osg::Vec4f c = image.getColor(x, y);
	return osg::Vec3f(c.x(), c.y(), c.z()) * 2.0 - osg::Vec3f(1.0, 1.0, 1.0);
}

TEST_CASE("Test normal map created from height map")
{
	std::vector<uint16_t> data = {
		1, 2, 3, 4,
		1, 2, 3, 4,
		1, 1, 4, 4,
		4, 4, 4, 4
	};

	osg::ref_ptr<osg::Image> heightMap = create4x4TestHeightImage();
	memcpy((void*)heightMap->getDataPointer(), data.data(), data.size() * sizeof(uint16_t));

	HeightMapElevationRerange rerange = {2, 123};
	osg::Vec2f texelWorldSize(3, 4);

	osg::ref_ptr<osg::Image> normalMap = createNormalMapFromHeightMap(*heightMap, rerange, texelWorldSize);

	// Test X sloping area
	{
		osg::Vec3f actualNormal = readNormal(*normalMap, 0, 0);
		osg::Vec3f expectedNormal(-2, 0, 3);
		expectedNormal.normalize();

		CHECK(almostEqual(expectedNormal, actualNormal, 0.01f));
	}

	// Test Y sloping area
	{
		osg::Vec3f actualNormal = readNormal(*normalMap, 0, 2);
		osg::Vec3f expectedNormal(0, -6, 4);
		expectedNormal.normalize();

		CHECK(almostEqual(expectedNormal, actualNormal, 0.01f));
	}

	// Test flat area
	{
		osg::Vec3f actualNormal = readNormal(*normalMap, 3, 3);
		osg::Vec3f expectedNormal(0, 0, 1);
		expectedNormal.normalize();

		CHECK(almostEqual(expectedNormal, actualNormal, 0.01f));
	}
}
