/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/Shadow/ShadowHelpers.h"
#include <osg/Texture2D>

#include <boost/optional.hpp>

namespace skybolt {
namespace vis {

struct DetailMappingTechnique
{
public:
	virtual ~DetailMappingTechnique() = default;
};

//! One detail map applied uniformly over the terrain
struct UniformDetailMappingTechnique : DetailMappingTechnique
{
	~UniformDetailMappingTechnique() override = default;

	osg::ref_ptr<osg::Texture2D> albedoDetailMap;
};

//! Multiple detail maps blended by coverage derived from albedo map
struct AlbedoDerivedDetailMappingTechnique : DetailMappingTechnique
{
	~AlbedoDerivedDetailMappingTechnique() override = default;

	osg::ref_ptr<osg::Texture2D> noiseMap;
	std::vector<osg::ref_ptr<osg::Texture2D>> albedoDetailMaps;
};

struct TerrainConfig
{
	struct Tile
	{
	protected:
		Tile() {}
		virtual ~Tile() {}
	};

	struct FlatTile : Tile
	{
		osg::Vec2f size;
	};

	struct PlanetTile : public Tile
	{
		Box2d latLonBounds;
		float planetRadius;
	};

	std::shared_ptr<Tile> tile;

	osg::ref_ptr<osg::Texture2D> heightMap;
	osg::ref_ptr<osg::Texture2D> normalMap;
	osg::ref_ptr<osg::Texture2D> landMask;
	osg::ref_ptr<osg::Texture2D> overallAlbedoMap;
	osg::ref_ptr<osg::Texture2D> attributeMap;

	DetailMappingTechniquePtr detailMappingTechnique; //!< May be null

	osg::ref_ptr<osg::Program> program;
	float heightScale = 65536;
	osg::Vec2f heightMapUvScale = osg::Vec2f(1, 1);
	osg::Vec2f heightMapUvOffset = osg::Vec2f(0, 0);
	osg::Vec2f overallAlbedoMapUvScale = osg::Vec2f(1, 1);
	osg::Vec2f overallAlbedoMapUvOffset = osg::Vec2f(0, 0);
	osg::Vec2f attributeMapUvScale = osg::Vec2f(1, 1);
	osg::Vec2f attributeMapUvOffset = osg::Vec2f(0, 0);
};

class Terrain : public DefaultRootNode
{
public:
	Terrain(const TerrainConfig& config);
	~Terrain();

	osg::Node* getTerrainNode() const { return mNode; }

	void updatePreRender(const RenderContext& context);

private:
	osg::Node* mNode;
};

} // namespace vis
} // namespace skybolt
