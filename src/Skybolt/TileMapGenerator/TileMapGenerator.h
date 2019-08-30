/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/Box2.h>
#include <osg/image>
#include <osg/Vec2i>

typedef skybolt::Box2T<osg::Vec2d> Box2d;

struct TileMapGeneratorLayer
{
	osg::ref_ptr<osg::Image> image;
	Box2d bounds;
};

enum class Filtering
{
	NearestNeighbor,
	Bilinear
};

//! Generates herichical tile map in XYZ format
void generateTileMap(const std::string& outputDirectory, const osg::Vec2i& tileDimensions, const std::vector<TileMapGeneratorLayer>& layers, Filtering filtering);
