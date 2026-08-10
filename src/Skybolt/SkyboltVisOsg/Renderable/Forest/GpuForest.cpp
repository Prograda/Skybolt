/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "GpuForest.h"

#include "SkyboltVisOsg/OsgGeocentric.h"
#include "SkyboltVisOsg/OsgGeometryFactory.h"
#include "SkyboltVisOsg/Renderable/Forest/BillboardForest.h"
#include "SkyboltVisOsg/Renderable/Forest/GpuForestTile.h"
#include "SkyboltVisOsg/Renderable/Forest/PagedForest.h"
#include "SkyboltVis/Renderable/Planet/Tile/TileKeyHelpers.h"
#include "SkyboltVisOsg/Shader/ShaderProgramRegistry.h"
#include <SkyboltCommon/Random.h>

#include <osg/Geode>

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
	mTileTexturesProvider(config.tileTexturesProvider),
	mParentGroup(config.parentGroup),
	mParentTransform(config.parentTransform),
	mPlanetRadius(config.planetRadius)
{
	assert(mPrograms);
	assert(mParentGroup);

	int lodCount = mForestParams.maxTileLodLevelToDisplayForest - mForestParams.minTileLodLevelToDisplayForest + 1;

	auto lonLatBounds = getKeyLonLatBounds<osg::Vec2>(QuadTreeTileKey(mForestParams.maxTileLodLevelToDisplayForest, 0, 0));
	double maxLodTileWidthMeters = lonLatBounds.size().x() * mPlanetRadius;
	int treeCountPerDimAtMaxLod = std::max(1, int(maxLodTileWidthMeters * mForestParams.treesPerLinearMeter));

	for (int i = 0; i < lodCount; ++i)
	{
		int repetitions = 1 << (lodCount - i);

		double tileWidthMeters = maxLodTileWidthMeters * (1 << i);

		auto forest = createBillboardForest(treeCountPerDimAtMaxLod, repetitions, osg::Vec2(tileWidthMeters, tileWidthMeters));
		mForestGeometries.push_back(forest);

#ifdef DRAW_BOUNDING_BOXES
		auto group = forest->_getNode()->asGroup();
		auto geode = group->getChild(1)->asGeode();
		auto drawable = createLineBox(geode->getBoundingBox());
		osg::StateSet* stateSet = drawable->getOrCreateStateSet();
		stateSet->setAttributeAndModes(mPrograms->getRequiredProgram("unlitColored"), osg::StateAttribute::ON);

		geode->addDrawable(drawable);
#endif
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

void removeTilesBelowLodLevel(TileKeyImagesMap& images, int minLodLevel)
{
	for (auto i = images.begin(); i != images.end(); )
	{
		if (i->first.level < minLodLevel)
		{
			i = images.erase(i);
		}
		else {
			++i;
		}
	}
}

void GpuForest::updateFromTree(const QuadTreeTileLoader::LoadedTileTree& tree)
{
	// Get required tile images
	TileKeyImagesMap requiredTileImages;
	findLeafTiles(tree, requiredTileImages, mForestParams.maxTileLodLevelToDisplayForest);
	removeTilesBelowLodLevel(requiredTileImages, mForestParams.minTileLodLevelToDisplayForest);

	// Removed current unreqiured forest tiles
	for (auto i = mForestTiles.begin(); i != mForestTiles.end();)
	{
		if (requiredTileImages.find(i->first) == requiredTileImages.end())
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
	for (const auto& [key, images] : requiredTileImages)
	{
		if (mForestTiles.find(key) == mForestTiles.end())
		{
			mForestTiles[key] = createForestTile(key, mTileTexturesProvider(*images));
		}
	}
}

// Quad UVs follow OpenGL standard where 0,0 is at bottom left of image,
// but we want origin to be at top left so that tiles appear right-side-up.
constexpr bool gpuForestTileFlipV = true;

GpuForest::ForestTile GpuForest::createForestTile(const QuadTreeTileKey& key, const GpuForestTileTextures& tile)
{
	assert(tile.attribute.texture);
	assert(tile.height.texture);

	glm::vec2 heightImageScale, heightImageOffset;
	getTileTransformInParentSpace(key, tile.height.key.level, heightImageScale, heightImageOffset, gpuForestTileFlipV);

	glm::vec2 attributeImageScale, attributeImageOffset;
	getTileTransformInParentSpace(key, tile.attribute.key.level, attributeImageScale, attributeImageOffset, gpuForestTileFlipV);

	auto lonLatBounds = getKeyLonLatBounds<osg::Vec2>(key);
	osg::Vec2d centerLatLon = math::vec2SwapComponents(lonLatBounds.center());
	osg::Vec3d tilePosition = llaToGeocentric(centerLatLon, 0, mPlanetRadius);

	double cosLat = cos(centerLatLon.x());

	osg::Matrix matrix;
	matrix.setTrans(tilePosition);
	osg::ref_ptr<osg::MatrixTransform> transform(new osg::MatrixTransform(matrix));

	osg::Vec2d latLonBoundsSize = lonLatBounds.size();
	osg::Vec2f tileWorldSizeMeters = osg::Vec2f(latLonBoundsSize.x() * mPlanetRadius, latLonBoundsSize.y() * mPlanetRadius * cosLat);

	// Select the appropriate forest geometry for the current LOD
	int forestGeoIndex = std::clamp(key.level - mForestParams.minTileLodLevelToDisplayForest, 0, int(mForestGeometries.size()) - 1);
	BillboardForestPtr forestGeo = mForestGeometries[forestGeoIndex];
	auto forestTile = std::make_shared<GpuForestTile>(tile.height.texture, tile.attribute.texture, forestGeo, tileWorldSizeMeters);

	{
		osg::StateSet* ss = forestTile->_getNode()->getOrCreateStateSet();
		ss->addUniform(new osg::Uniform("heightMapUvScale", math::toOsgVec2f(heightImageScale)));
		ss->addUniform(new osg::Uniform("heightMapUvOffset", math::toOsgVec2f(heightImageOffset)));
		ss->addUniform(new osg::Uniform("heightScale", tile.elevationRerange.x * 65535.f));
		ss->addUniform(new osg::Uniform("heightOffset", tile.elevationRerange.y));

		ss->addUniform(new osg::Uniform("attributeMapUvScale", math::toOsgVec2f(attributeImageScale)));
		ss->addUniform(new osg::Uniform("attributeMapUvOffset", math::toOsgVec2f(attributeImageOffset)));
	}
	mParentGroup->addChild(forestTile->_getNode());

	GpuForest::ForestTile result;
	result.tile = forestTile;
	result.tileCenter = centerLatLon;
	return result;
}

void GpuForest::updatePreRender(const CameraRenderContext& context)
{
	for (const auto& [key, tile] : mForestTiles)
	{
		osg::Vec3d tilePos = llaToGeocentric(tile.tileCenter, 0, mPlanetRadius) * mParentTransform->getMatrix();
		tilePos.z() = mParentTransform->getMatrix().getTrans().z() - mPlanetRadius;
		tile.tile->setPosition(tilePos);
		tile.tile->updatePreRender(context); // TODO: the Scene should call this, not the PlanetSurface
	}
}

std::shared_ptr<BillboardForest> GpuForest::createBillboardForest(int treeCountPerDimension, int repetitions, const osg::Vec2& tileBoundsMeters) const
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
	return std::make_shared<BillboardForest>(trees, mPrograms->getRequiredProgram("treeSideBillboard"), mPrograms->getRequiredProgram("treeTopBillboard"), mForestParams.forestGeoVisibilityRange, tileBoundsMeters, subTileCount);
}

} // namespace vis
} // namespace skybolt
