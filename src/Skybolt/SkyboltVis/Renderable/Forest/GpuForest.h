#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
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

struct GpuForestConfig
{
	ForestParams forestParams;
	const ShaderPrograms* programs;
	osg::ref_ptr<osg::Group> parentGroup;
	osg::ref_ptr<osg::MatrixTransform> parentTransform;
	double planetRadius;
};

class GpuForest
{
public:
	GpuForest(const GpuForestConfig& config);
	~GpuForest();

	struct TileTextures
	{
		TileTexture height;
		TileTexture attribute;
		QuadTreeTileKey key;
	};

	void terrainTilesUpdated(const std::vector<TileTextures>& addedTiles, const std::vector<QuadTreeTileKey>& removedTiles);

	void updatePreRender(const RenderContext& context);

private:
	struct ForestTile
	{
		GpuForestTilePtr tile;
		osg::Vec2d tileCenter;
	};

	ForestTile createForestTile(const TileTextures& tile);

	std::shared_ptr<BillboardForest> createBillboardForest(int treeCountPerDimension, int repetitions) const;

private:
	const ForestParams mForestParams;
	const ShaderPrograms* mPrograms;
	osg::ref_ptr<osg::Group> mParentGroup;
	osg::ref_ptr<osg::MatrixTransform> mParentTransform;
	double mPlanetRadius;

	//! Defines a vector of pre-defined forest tile geometries at increasing tiling repetitions.
	//! The first geometry has no repetition, the second geometry repeats twice in each dimension,
	//! the third geometry repeats 4 times in each dimension etc. This allows terrain tiles at different
	//! LODs to have tree pacement patterns that match.
	std::vector<std::shared_ptr<BillboardForest>> mForestGeometries;

	std::map<QuadTreeTileKey, ForestTile> mForestTiles;
	std::set<QuadTreeTileKey> mTerrainTiles;
};

} // namespace vis
} // namespace skybolt
