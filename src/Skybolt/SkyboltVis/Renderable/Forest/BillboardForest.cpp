/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BillboardForest.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgGeometryHelpers.h"
#include "SkyboltVis/OsgTextureHelpers.h"
#include "SkyboltVis/VisibilityCategory.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureBuffer>

namespace skybolt {
namespace vis {

osg::Geometry* createGeometry(const std::vector<BillboardForest::Tree>& trees, const osg::Vec2& tileBoundsMeters)
{
    osg::Geometry *geometry = new osg::Geometry();

	// TODO: optimize using instancing
	osg::Vec3Array* vertPositions = new osg::Vec3Array;
	vertPositions->resize(trees.size() * 4);

	int vertexIndex = 0;
	for (int i = 0; i < trees.size(); ++i)
	{
		const BillboardForest::Tree& tree = trees[i];

		for (int j = 0; j < 4; ++j)
		{
			(*vertPositions)[vertexIndex] = tree.position;
			++vertexIndex;
		}
	}

    geometry->setVertexArray(vertPositions);
	vis::configureDrawable(*geometry);

	double maxForestAltitude = 9000;
	osg::Vec3 halfBoundsWidth(tileBoundsMeters.x() * 0.5, tileBoundsMeters.y() * 0.5, maxForestAltitude);
	geometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(osg::BoundingBox(-halfBoundsWidth, halfBoundsWidth)));

    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertPositions->size()));

    return geometry;
}

struct Uniforms
{
	osg::Uniform* maxVisibilityRange;
};

osg::StateSet* createStateSet(osg::ref_ptr<osg::Program> program, const Uniforms& uniforms, const std::vector<BillboardForest::Tree>& trees, int subTileCount)
{
	osg::StateSet* ss = new osg::StateSet;

	ss->setAttributeAndModes(program, osg::StateAttribute::ON);
	ss->addUniform(uniforms.maxVisibilityRange);
	ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

	// TODO: get rid of static and cache the texture to share for every BillboardForest instance. Try to put the TBO part in a subgraph that's different for each page, and share the rest in a common parent. See https://github.com/openscenegraph/OpenSceneGraph/blob/master/examples/osgforest/osgforest.cpp
	static osg::ref_ptr<osg::Texture2D> albedoTexture = createSrgbTexture(readImageWithCorrectOrientation("Environment/Forest/spruceAtlas_side_albedo.tga"));
	albedoTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	albedoTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	ss->setTextureAttributeAndModes(0, albedoTexture, osg::StateAttribute::ON);

	// Create texture to store tree parameters
	int treeCount = (int)trees.size() / subTileCount;
	osg::ref_ptr<osg::Image> treeParamsImage = new osg::Image;
	treeParamsImage->allocateImage(treeCount, 1, 1, GL_RGBA, GL_FLOAT); // TODO: save memory be using GL_RGB. The alpha channel is unused.

	osg::Vec4f* ptr = (osg::Vec4f*)treeParamsImage->data();
	for (int i = 0; i < treeCount; ++i)
	{
		const BillboardForest::Tree& tree = trees[i];
		*ptr = osg::Vec4f(tree.type, tree.height, tree.yaw, 0.f);
		++ptr;
	}

	osg::TextureBuffer* treeParamsTexture = new osg::TextureBuffer;
	treeParamsTexture->setImage(treeParamsImage);
	treeParamsTexture->setInternalFormat(GL_RGBA32F_ARB);
	ss->setTextureAttribute(1, treeParamsTexture, osg::StateAttribute::ON);

	ss->addUniform(createUniformSampler2d("albedoSampler", 0));
	ss->addUniform(createUniformSamplerTbo("treeParamsSampler", 1));

	return ss;
}

osg::Geode* createGeode(osg::Geometry* geometry)
{
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(geometry);
	geode->setCullingActive(true);
	return geode;
}

BillboardForest::BillboardForest(const std::vector<Tree>& trees, osg::ref_ptr<osg::Program> sideProgram, osg::ref_ptr<osg::Program> topProgram, float maxVisibilityRange, const osg::Vec2& tileBoundsMeters, int subTileCount)
{
	mTransform->setNodeMask(vis::VisibilityCategory::defaultCategories | vis::VisibilityCategory::shadowCaster);
	addGeodes(*mTransform, trees, sideProgram, topProgram, maxVisibilityRange, tileBoundsMeters, subTileCount);
}

void BillboardForest::addGeodes(osg::Group& node, const std::vector<Tree>& trees, osg::ref_ptr<osg::Program> sideProgram, osg::ref_ptr<osg::Program> topProgram, float maxVisibilityRange, const osg::Vec2& tileBoundsMeters, int subTileCount)
{
	Uniforms uniforms;
	uniforms.maxVisibilityRange = new osg::Uniform("maxVisibilityRange", maxVisibilityRange);

	osg::Geometry* geometry = createGeometry(trees, tileBoundsMeters);

	osg::Geode* sideGeode = createGeode(geometry);
	sideGeode->setStateSet(createStateSet(sideProgram, uniforms, trees, subTileCount));
	node.addChild(sideGeode);

#ifdef TREE_TOP_VIEW_BILLBOARDS
	osg::Geode* topGeode = createGeode(geometry);
	osg::StateSet* ss = new osg::StateSet(*sideGeode->getStateSet(), osg::CopyOp::SHALLOW_COPY);
	ss->setAttributeAndModes(topProgram, osg::StateAttribute::ON);
	static osg::ref_ptr<osg::Texture2D> albedoTexture = new osg::Texture2D(readImageWithCorrectOrientation("Environment/Forest/spruceAtlas_top_albedo.tga"));
	albedoTexture->setInternalFormat(toSrgbInternalFormat(albedoTexture->getInternalFormat()));
	ss->setTextureAttributeAndModes(0, albedoTexture, osg::StateAttribute::ON);
	// TODO: Modify alpha channel of mipmaps to achieve consistent coverage

	topGeode->setStateSet(ss);
	node.addChild(topGeode);
#endif
}

} // namespace vis
} // namespace skybolt
