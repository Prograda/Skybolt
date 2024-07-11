/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltCommon/Math/QuadTreeUtility.h>

#include <osg/Vec2>

using namespace skybolt;

typedef DefaultTile<osg::Vec2> Tile;

TEST_CASE("createAncestorKey")
{
	CHECK(createAncestorKey(QuadTreeTileKey(1, 0, 1), 1) == QuadTreeTileKey(1, 0, 1));
	CHECK(createAncestorKey(QuadTreeTileKey(1, 0, 1), 0) == QuadTreeTileKey(0, 0, 0));
	CHECK(createAncestorKey(QuadTreeTileKey(2, 2, 2), 1) == QuadTreeTileKey(1, 1, 1));
}

TEST_CASE("getKeyLatLonBounds")
{
	QuadTreeTileKey key;
	key.level = 1;
	key.x = 1;
	key.y = 0;

	float epsilon = 1e-7f;
	auto bounds = getKeyLonLatBounds<osg::Vec2>(key);
	CHECK(bounds.minimum.x() == Approx(-math::halfPiF()).margin(epsilon));
	CHECK(bounds.maximum.x() == Approx(0).margin(epsilon));
	CHECK(bounds.minimum.y() == Approx(0).margin(epsilon));
	CHECK(bounds.maximum.y() == Approx(math::halfPiF()).margin(epsilon));
}


TEST_CASE("getKeyLonLatBounds")
{
	QuadTreeTileKey key;
	key.level = 1;
	key.x = 1;
	key.y = 0;

	auto bounds = getKeyLonLatBounds<osg::Vec2>(key);
	auto boundsSwapped = getKeyLatLonBounds<osg::Vec2>(key);
	CHECK(bounds.minimum.x() == boundsSwapped.minimum.y());
	CHECK(bounds.maximum.x() == boundsSwapped.maximum.y());
	CHECK(bounds.minimum.y() == boundsSwapped.minimum.x());
	CHECK(bounds.maximum.y() == boundsSwapped.maximum.x());
}

TEST_CASE("getKeyAtLevelIntersectingPoint returns key that contains the point")
{
	osg::Vec2 point(0.234f, 0.567f);
	QuadTreeTileKey key = getKeyAtLevelIntersectingLonLatPoint(4, point);
	auto bounds = getKeyLonLatBounds<osg::Vec2>(key);
	CHECK(bounds.intersects(point));
}