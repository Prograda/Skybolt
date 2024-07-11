/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>

#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <SkyboltVis/TextureGenerator/GpuTextureGenerator.h>
#include "SkyboltVis/TextureGenerator/GpuTextureGeneratorStateSets.h"
#include <SkyboltVis/Window/OffscreenViewer.h>

#include <osgDB/WriteFile>

using namespace skybolt::vis;

TEST_CASE("Generate normal map from vector displacement map")
{
	// TODO: reenable and assert output image is as expected
	/*
	int width = 64;
	int height = 64;

	// Create input texture
	osg::Image* inputImage = new osg::Image();
	inputImage->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE);
	//inputImage->setInternalTextureFormat(GL_RGB32F_ARB);
	
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			inputImage->setColor(osg::Vec4(std::abs(32-x)/32.0, std::abs(32-y) / 32.0, 0, 1), x, y);
		}
	}

	osg::Texture2D* inputTexture = new osg::Texture2D(inputImage);

	// Create output texture
	osg::Image* outputImage = new osg::Image();
	outputImage->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE);
	osg::Texture2D* outputTexture = new osg::Texture2D(outputImage);

	// Create generator
	ShaderPrograms programs = createShaderPrograms();

	osg::Vec2 textureScale(10, 10);
	osg::ref_ptr<GpuTextureGenerator> generator = new GpuTextureGenerator(outputTexture, createVectorDisplacementToNormalMapStateSet(programs.vectorDisplacementToNormal, inputTexture, textureScale), true);
	
	osg::ref_ptr<osgViewer::Viewer> viewer = createOffscreenViewer(width, height);
	viewer->getCamera()->addChild(generator);

	// Generate
	viewer->frame();

	//osgDB::writeImageFile(*outputTexture->getImage(), "C:/Users/Public/test.tga");
	*/
}
