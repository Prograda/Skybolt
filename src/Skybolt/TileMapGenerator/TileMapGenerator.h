/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/OsgBox2.h>
#include <osg/Image>
#include <osg/Vec2i>

struct TileMapGeneratorLayer
{
	osg::ref_ptr<osg::Image> image;
	skybolt::vis::Box2d bounds;
};

enum class Filtering
{
	NearestNeighbor,
	Bilinear
};

//! Generates herichical tile map in XYZ format
void generateTileMap(const std::string& outputDirectory, const osg::Vec2i& tileDimensions, const std::vector<TileMapGeneratorLayer>& layers, Filtering filtering);
