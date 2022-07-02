/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Renderable/Planet/Tile/QuadTreeTileLoader.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileTexture.h"
#include <SkyboltCommon/Math/QuadTree.h>
#include <osg/Group>
#include <optional>

namespace skybolt {
namespace vis {

struct ForestParams
{
	float forestGeoVisibilityRange = 8000.f;
	float treesPerLinearMeter = 0.01f;
	int minTileLodLevelToDisplayForest = 12; //!< Don't show forest on tiles below this level
	int maxTileLodLevelToDisplayForest = 14; //!< Don't show forest on tiles above this level
};

struct GpuForestTileTextures
{
	TileTexture height;
	TileTexture attribute;
};

struct GpuForestConfig
{
	ForestParams forestParams;
	const ShaderPrograms* programs;
	std::function<GpuForestTileTextures(const TileImages&)> tileTexturesProvider;
	osg::ref_ptr<osg::Group> parentGroup;
	osg::ref_ptr<osg::MatrixTransform> parentTransform;
	double planetRadius;
};

class GpuForest
{
public:
	GpuForest(const GpuForestConfig& config);
	~GpuForest();

	void updateFromTree(const QuadTreeTileLoader::LoadedTileTree& tree);

	void updatePreRender(const CameraRenderContext& context);

private:
	struct ForestTile
	{
		GpuForestTilePtr tile;
		osg::Vec2d tileCenter;
	};

	ForestTile createForestTile(const QuadTreeTileKey& key, const GpuForestTileTextures& tile);

	std::shared_ptr<BillboardForest> createBillboardForest(int treeCountPerDimension, int repetitions, const osg::Vec2& tileBoundsMeters) const;

private:
	const ForestParams mForestParams;
	const ShaderPrograms* mPrograms;
	std::function<GpuForestTileTextures(const TileImages&)> mTileTexturesProvider;
	osg::ref_ptr<osg::Group> mParentGroup;
	osg::ref_ptr<osg::MatrixTransform> mParentTransform;
	double mPlanetRadius;

	//! Defines a vector of pre-defined forest tile geometries at increasing tiling repetitions.
	//! The first geometry has no repetition, the second geometry repeats twice in each dimension,
	//! the third geometry repeats 4 times in each dimension etc. This allows terrain tiles at different
	//! LODs to have tree pacement patterns that match.
	std::vector<std::shared_ptr<BillboardForest>> mForestGeometries;

	std::map<QuadTreeTileKey, ForestTile> mForestTiles;
};

} // namespace vis
} // namespace skybolt
