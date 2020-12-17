/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BillboardForest.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include "SkyboltVis/Camera.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureBuffer>

namespace skybolt {
namespace vis {

class BoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
	osg::BoundingBox computeBound(const osg::Drawable & drawable)
	{// TODO: compute actual bound
		return osg::BoundingBox(osg::Vec3f(-FLT_MAX, -FLT_MAX, 0), osg::Vec3f(FLT_MAX, FLT_MAX, 0));
	}
};

osg::Geometry* createGeometry(const std::vector<BillboardForest::Tree>& trees)
{
    osg::Geometry *geometry = new osg::Geometry();

	// TODO: optimize using instancing
	osg::Vec3Array* vertPositions = new osg::Vec3Array;

	for (int i = 0; i < trees.size(); ++i)
	{
		const BillboardForest::Tree& tree = trees[i];

		for (int j = 0; j < 4; ++j)
		{
			vertPositions->push_back(tree.position);
		}
	}

    geometry->setVertexArray(vertPositions);
	geometry->setUseDisplayList(false); 
    geometry->setUseVertexBufferObjects(true); 
	geometry->setUseVertexArrayObject(true);
	geometry->setComputeBoundingBoxCallback(osg::ref_ptr<BoundingBoxCallback>(new BoundingBoxCallback));

    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertPositions->size()));

    return geometry;
}

struct Uniforms
{
	osg::Uniform* maxVisibilityRange;
};

osg::StateSet* createStateSet(osg::ref_ptr<osg::Program> program, const Uniforms& uniforms, const std::vector<BillboardForest::Tree>& trees)
{
	osg::StateSet* ss = new osg::StateSet;

	ss->setAttributeAndModes(program, osg::StateAttribute::ON);
	ss->addUniform(uniforms.maxVisibilityRange);
	ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

	// TODO: get rid of static and cache the texture to share for every BillboardForest instance. Try to put the TBO part in a subgraph that's different for each page, and share the rest in a common parent. See https://github.com/openscenegraph/OpenSceneGraph/blob/master/examples/osgforest/osgforest.cpp
	static osg::ref_ptr<osg::Texture2D> albedoTexture = new osg::Texture2D(readImageWithCorrectOrientation("Environment/Forest/spruceAtlas_side_albedo.tga"));
	albedoTexture->setInternalFormat(toSrgbInternalFormat(albedoTexture->getInternalFormat()));
	albedoTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	albedoTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	ss->setTextureAttributeAndModes(0, albedoTexture, osg::StateAttribute::ON);

	// Create texture to store tree parameters
	osg::ref_ptr<osg::Image> treeParamsImage = new osg::Image;
	treeParamsImage->allocateImage(trees.size(), 1, 1, GL_RGBA, GL_FLOAT); // TODO: save memory be using GL_RGB. The alpha channel is unused.

	osg::Vec4f* ptr = (osg::Vec4f*)treeParamsImage->data();
	for (const BillboardForest::Tree& tree : trees)
	{
		*ptr = osg::Vec4f(tree.type, tree.height, tree.yaw, 0.f);
		++ptr;
	}

	osg::TextureBuffer* treeParamsTexture = new osg::TextureBuffer;
	treeParamsTexture->setImage(treeParamsImage);
	treeParamsTexture->setInternalFormat(GL_RGBA32F_ARB);
	ss->setTextureAttributeAndModes(1, treeParamsTexture, osg::StateAttribute::ON);

	ss->addUniform(createUniformSampler2d("albedoSampler", 0));
	ss->addUniform(createUniformSamplerTbo("treeParamsSampler", 1));

	return ss;
}

osg::Geode* createGeode(osg::Geometry* geometry)
{
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(geometry);
	geode->setCullingActive(false);
	return geode;
}

BillboardForest::BillboardForest(const std::vector<Tree>& trees, osg::ref_ptr<osg::Program> sideProgram, osg::ref_ptr<osg::Program> topProgram, float maxVisibilityRange)
{
	addGeodes(*mTransform, trees, sideProgram, topProgram, maxVisibilityRange);
}

void BillboardForest::addGeodes(osg::Group& node, const std::vector<Tree>& trees, osg::ref_ptr<osg::Program> sideProgram, osg::ref_ptr<osg::Program> topProgram, float maxVisibilityRange)
{
	Uniforms uniforms;
	uniforms.maxVisibilityRange = new osg::Uniform("maxVisibilityRange", maxVisibilityRange);

	osg::Geometry* geometry = createGeometry(trees);

	osg::Geode* sideGeode = createGeode(geometry);
	sideGeode->setStateSet(createStateSet(sideProgram, uniforms, trees));
	node.addChild(sideGeode);

	osg::Geode* topGeode = createGeode(geometry);
	osg::StateSet* ss = new osg::StateSet(*sideGeode->getStateSet(), osg::CopyOp::SHALLOW_COPY);
	ss->setAttributeAndModes(topProgram, osg::StateAttribute::ON);
	static osg::ref_ptr<osg::Texture2D> albedoTexture = new osg::Texture2D(readImageWithCorrectOrientation("Environment/Forest/spruceAtlas_top_albedo.tga"));
	albedoTexture->setInternalFormat(toSrgbInternalFormat(albedoTexture->getInternalFormat()));
	ss->setTextureAttributeAndModes(0, albedoTexture, osg::StateAttribute::ON);
	// TODO: Modify alpha channel of mipmaps to achieve consistent coverage

	topGeode->setStateSet(ss);
	node.addChild(topGeode);
}

} // namespace vis
} // namespace skybolt
