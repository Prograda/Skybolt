#include "GpuForest.h"

#include "SkyboltVis/OsgGeocentric.h"
#include "SkyboltVis/Renderable/Forest/BillboardForest.h"
#include "SkyboltVis/Renderable/Forest/GpuForestTile.h"
#include "SkyboltVis/Renderable/Forest/PagedForest.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileKeyHelpers.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"
#include <SkyboltCommon/Random.h>

namespace skybolt {
namespace vis {

std::vector<float> treeTypeHeights = {45, 45, 45};

static void generate(std::vector<BillboardForest::Tree>& trees, int treeCountPerDimension, const osg::Vec2f& scale, const osg::Vec2f& offset)
{
	skybolt::Random random(0);

	for (int y = 0; y < treeCountPerDimension; ++y)
	{
		for (int x = 0; x < treeCountPerDimension; ++x)
		{
			BillboardForest::Tree tree;
			tree.position.x() = float(x + random.unitRand()) / treeCountPerDimension * scale.x() + offset.x();
			tree.position.y() = float(y + random.unitRand()) / treeCountPerDimension * scale.y() + offset.y();
			tree.position.z() = 0;
			tree.type = std::min((int)treeTypeHeights.size() - 1, int(random.unitRand() * treeTypeHeights.size()));
			tree.height = treeTypeHeights[tree.type] * skybolt::math::lerp(0.75f, 1.25f, random.unitRand());
			tree.yaw = 2.f * osg::PI * random.unitRand();
			trees.push_back(tree);
		}
	}
}

GpuForest::GpuForest(const GpuForestConfig& config) :
	mForestParams(config.forestParams),
	mPrograms(config.programs),
	mParentGroup(config.parentGroup),
	mParentTransform(config.parentTransform),
	mPlanetRadius(config.planetRadius)
{
	assert(mPrograms);
	assert(mParentGroup);

	int lodCount = mForestParams.maxTileLodLevelToDisplayForest - mForestParams.minTileLodLevelToDisplayForest + 1;

	auto lonLatBounds = getKeyLonLatBounds<osg::Vec2>(QuadTreeTileKey(mForestParams.maxTileLodLevelToDisplayForest, 0, 0));
	double tileWidthMeters = lonLatBounds.size().x() * mPlanetRadius;
	int treeCountPerDimAtMaxLod = std::max(1, int(tileWidthMeters * mForestParams.treesPerLinearMeter));

	for (int i = 0; i < lodCount; ++i)
	{
		int repetitions = 1 << (lodCount - i);
		mForestGeometries.push_back(createBillboardForest(treeCountPerDimAtMaxLod, repetitions));
	}
}

GpuForest::~GpuForest()
{
	// Remove forest OSG nodes
	for (const auto& [key, tile] : mForestTiles)
	{
		mParentGroup->removeChild(tile.tile->_getNode());
	}
}

void GpuForest::terrainTilesUpdated(const std::vector<TileTextures>& addedTiles, const std::vector<QuadTreeTileKey>& removedTiles)
{
	// Update set of current terrain tile keys
	for (const auto& tile : removedTiles)
	{
		mTerrainTiles.erase(tile);
	}

	for (const auto& tile : addedTiles)
	{
		mTerrainTiles.insert(tile.key);
	}

	// Get current required forest tile keys
	std::set<QuadTreeTileKey> requiredForestKeys;
	for (const auto& tile : mTerrainTiles)
	{
		if (tile.level > mForestParams.maxTileLodLevelToDisplayForest)
		{
			requiredForestKeys.insert(createAncestorKey(tile, mForestParams.maxTileLodLevelToDisplayForest));
		}
		else if (tile.level >= mForestParams.minTileLodLevelToDisplayForest)
		{
			requiredForestKeys.insert(tile);
		}
	}

	// Removed current unreqiured forest tiles
	for (auto i = mForestTiles.begin(); i != mForestTiles.end();)
	{
		if (requiredForestKeys.find(i->first) == requiredForestKeys.end())
		{
			auto itToRemove = i;
			++i;
			mParentGroup->removeChild(itToRemove->second.tile->_getNode());
			mForestTiles.erase(itToRemove);
		}
		else
		{
			++i;
		}
	}

	// Add new required forest tiles
	for (const auto& tile : addedTiles)
	{
		if (requiredForestKeys.find(tile.key) != requiredForestKeys.end())
		{
			if (mForestTiles.find(tile.key) == mForestTiles.end())
			{
				mForestTiles[tile.key] = createForestTile(tile);
			}
		}
	}
}

GpuForest::ForestTile GpuForest::createForestTile(const TileTextures& tile)
{
	assert(tile.attribute.texture);
	assert(tile.height.texture);

	osg::Vec2f heightImageScale, heightImageOffset;
	getTileTransformInParentSpace(tile.key, tile.height.key.level, heightImageScale, heightImageOffset);

	osg::Vec2f attributeImageScale, attributeImageOffset;
	getTileTransformInParentSpace(tile.key, tile.attribute.key.level, attributeImageScale, attributeImageOffset);

	auto lonLatBounds = getKeyLonLatBounds<osg::Vec2>(tile.key);
	osg::Vec2d centerLatLon = math::vec2SwapComponents(lonLatBounds.center());
	osg::Vec3d tilePosition = llaToGeocentric(centerLatLon, 0, mPlanetRadius);

	double cosLat = cos(centerLatLon.x());

	osg::Matrix matrix;
	matrix.setTrans(tilePosition);
	osg::ref_ptr<osg::MatrixTransform> transform(new osg::MatrixTransform(matrix));

	osg::Vec2d latLonBoundsSize = lonLatBounds.size();
	osg::Vec2f tileWorldSizeMeters = osg::Vec2f(latLonBoundsSize.x() * mPlanetRadius, latLonBoundsSize.y() * mPlanetRadius * cosLat);

	// Select the appropriate forest geometry for the current LOD
	int forestGeoIndex = std::clamp(tile.key.level - mForestParams.minTileLodLevelToDisplayForest, 0, int(mForestGeometries.size()) - 1);
	BillboardForestPtr forestGeo = mForestGeometries[forestGeoIndex];
	auto forestTile = std::make_shared<GpuForestTile>(tile.height.texture, tile.attribute.texture, forestGeo, tileWorldSizeMeters);

	{
		osg::StateSet* ss = forestTile->_getNode()->getOrCreateStateSet();
		ss->addUniform(new osg::Uniform("heightMapUvScale", heightImageScale));
		ss->addUniform(new osg::Uniform("heightMapUvOffset", heightImageOffset));
		ss->addUniform(new osg::Uniform("heightScale", 65536.f));

		ss->addUniform(new osg::Uniform("attributeMapUvScale", attributeImageScale));
		ss->addUniform(new osg::Uniform("attributeMapUvOffset", attributeImageOffset));
	}
	mParentGroup->addChild(forestTile->_getNode());

	GpuForest::ForestTile result;
	result.tile = forestTile;
	result.tileCenter = centerLatLon;
	return result;
}

void GpuForest::updatePreRender(const RenderContext& context)
{
	for (const auto& [key, tile] : mForestTiles)
	{
		osg::Vec3d tilePos = llaToGeocentric(tile.tileCenter, 0, mPlanetRadius) * mParentTransform->getMatrix();
		tilePos.z() = mParentTransform->getMatrix().getTrans().z() - mPlanetRadius;
		tile.tile->setPosition(tilePos);
		tile.tile->updatePreRender(context); // TODO: the Scene should call this, not the PlanetSurface
	}
}

std::shared_ptr<BillboardForest> GpuForest::createBillboardForest(int treeCountPerDimension, int repetitions) const
{
	assert(repetitions >= 1);

	std::vector<BillboardForest::Tree> trees;

	osg::Vec2f scale(1.0f / repetitions, 1.0f / repetitions);
	for (int y = 0; y < repetitions; ++y)
	{
		for (int x = 0; x < repetitions; ++x)
		{
			osg::Vec2f offset(x * scale.x(), y * scale.y());
			generate(trees, treeCountPerDimension, scale, offset);
		}
	}

	int subTileCount = repetitions * repetitions;
	return std::make_shared<BillboardForest>(trees, mPrograms->getRequiredProgram("treeSideBillboard"), mPrograms->getRequiredProgram("treeTopBillboard"), mForestParams.forestGeoVisibilityRange, subTileCount);
}

} // namespace vis
} // namespace skybolt
