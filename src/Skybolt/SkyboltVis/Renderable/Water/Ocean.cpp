/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Ocean.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/OsgGeometryFactory.h"
#include "SkyboltVis/OsgGeometryHelpers.h"
#include "SkyboltVis/RenderContext.h"

#include <osg/CullFace>
#include <osg/Geode>
#include <osg/Geometry>

//#define WIREFRAME
#ifdef WIREFRAME
#include <osg/PolygonMode>
#endif

using namespace skybolt::vis;

osg::Node* createPlane(const osg::Vec2f &size)
{
	int segmentCountX = 512;
	int segmentCountY = 512;
	
	osg::Vec3Array* posBuffer = new osg::Vec3Array();
	osg::UIntArray* indexBuffer = new osg::UIntArray();

	createPlaneBuffers(*posBuffer, *indexBuffer, -size * 0.5f, size, segmentCountX, segmentCountY, Triangles);

    osg::Geode *geode = new osg::Geode();
    osg::Geometry *geometry = new osg::Geometry();

    geometry->setVertexArray(posBuffer);
	configureDrawable(*geometry);
	geometry->setCullingActive(false);

    geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));
    geode->addDrawable(geometry);
	geode->setCullingActive(false);
    return geode;
}

osg::StateSet* createStateSet(const OceanConfig& config, const Ocean::Uniforms& uniforms)
{
	osg::StateSet* stateSet = new osg::StateSet();
	stateSet->setDefine("DISTANCE_CULL");

	stateSet->setAttributeAndModes(config.oceanProgram, osg::StateAttribute::ON);
	stateSet->addUniform(uniforms.topLeftDir);
	stateSet->addUniform(uniforms.topRightDir);
	stateSet->addUniform(uniforms.bottomLeftDir);
	stateSet->addUniform(uniforms.bottomRightDir);
	stateSet->addUniform(uniforms.waterHeight);

	stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateSet->setRenderBinDetails(1, "RenderBin");

#ifdef WIREFRAME
	osg::PolygonMode * polygonMode = new osg::PolygonMode;
	polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );

	stateSet->setAttributeAndModes( polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
#endif
   return stateSet;
}

Ocean::Ocean(const OceanConfig& config)
{
	mUniforms.topLeftDir = new osg::Uniform("topLeftDir", osg::Vec3f(0,0,0));
	mUniforms.topRightDir = new osg::Uniform("topRightDir", osg::Vec3f(0,0,0));
	mUniforms.bottomLeftDir = new osg::Uniform("bottomLeftDir", osg::Vec3f(0,0,0));
	mUniforms.bottomRightDir = new osg::Uniform("bottomRightDir", osg::Vec3f(0,0,0));
	mUniforms.waterHeight = new osg::Uniform("waterHeight", 0.0f);

	const osg::Vec2f size(1.0f, 1.0f);
	mGrid = createPlane(size);
	mGrid->setStateSet(createStateSet(config, mUniforms));

	mTransform->setStateSet(config.waterStateSet);
    mTransform->addChild(mGrid);

	setPosition(osg::Vec3f(0,0,0));
}

Ocean::~Ocean()
{
	mTransform->removeChild(mGrid);
}

void Ocean::setPosition(const osg::Vec3f& position)
{
	mUniforms.waterHeight->set(float(-position.z()));
}

void Ocean::updatePreRender(const RenderContext& context)
{
	const Camera& camera = context.camera;
	// Update uniforms
	{
		osg::Vec3f cameraPosition = camera.getPosition();

		osg::Matrixf viewProj = camera.getViewMatrix() * camera.getProjectionMatrix();
		osg::Matrixf viewProjInv = osg::Matrix::inverse(viewProj);

		osg::Vec4f c00 = osg::Vec4f(-1, -1, 0.5f, 1.0f) * viewProjInv;
		c00 /= c00.w();
		osg::Vec4f c10 = osg::Vec4f(1, -1, 0.5f, 1.0f) * viewProjInv;
		c10 /= c10.w();
		osg::Vec4f c01 = osg::Vec4f(-1, 1, 0.5f, 1.0f) * viewProjInv;
		c01 /= c01.w();
		osg::Vec4f c11 = osg::Vec4f(1, 1, 0.5f, 1.0f) * viewProjInv;
		c11 /= c11.w();
	
		osg::Vec3f dir00 = osg::Vec3f(c00.x(), c00.y(), c00.z()) - cameraPosition;
		dir00.normalize();
		osg::Vec3f dir10 = osg::Vec3f(c10.x(), c10.y(), c10.z()) - cameraPosition;
		dir10.normalize();
		osg::Vec3f dir01 = osg::Vec3f(c01.x(), c01.y(), c01.z()) - cameraPosition;
		dir01.normalize();
		osg::Vec3f dir11 = osg::Vec3f(c11.x(), c11.y(), c11.z()) - cameraPosition;
		dir11.normalize();

		mUniforms.topLeftDir->set(dir00);
		mUniforms.topRightDir->set(dir10);
		mUniforms.bottomLeftDir->set(dir01);
		mUniforms.bottomRightDir->set(dir11);
	}
}
