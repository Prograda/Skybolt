/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltVis/Renderable/Forest/BillboardTextureGenerator.h>
#include <SkyboltVis/Renderable/Model/ModelPreparer.h>
#include <SkyboltVis/Window/OffscreenViewer.h>

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace skybolt;
using namespace skybolt::vis;

class ImposterGenerator
{
public:
	struct Result
	{
		osg::ref_ptr<osg::Image> albedoMap;
		osg::ref_ptr<osg::Image> normalMap;
	};

	ImposterGenerator(const std::string& modelFilename, const osg::Vec2i& imageSize) :
		mImageSize(imageSize)
	{
		osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(modelFilename);
		model->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

		mTransform = new osg::MatrixTransform;
		mTransform->addChild(model);

		ModelPreparerConfig config;
		ModelPreparer preparer(config);
		preparer.apply(*model);
	}

	Result render(float yaw, float pitch, BillboardTextureFitMode fitMode)
	{
		osg::Matrix mat = mTransform->getMatrix();
		mat.setRotate(osg::Quat(osg::PI, osg::Vec3d(1, 0, 0)));
		mTransform->setMatrix(mat);

		osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
		viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
		viewer->getCamera()->setGraphicsContext(createOffscreenContext(mImageSize.x(), mImageSize.y()));
		viewer->getCamera()->setRenderTargetImplementation(osg::Camera::PIXEL_BUFFER);
		viewer->getCamera()->setViewport(new osg::Viewport(0, 0, mImageSize.x(), mImageSize.y()));

		osg::State* state = viewer->getCamera()->getGraphicsContext()->getState();
		state->setUseModelViewAndProjectionUniforms(true);
		state->setUseVertexAttributeAliasing(true);
		mTransform->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

		osg::Vec3f cameraForwardDir = osg::Quat(yaw, osg::Vec3d(0, 0, -1)) * osg::Quat(pitch, osg::Vec3d(0, -1, 0)) * osg::Vec3f(1, 0, 0);
		BillboardTextureCamera camera = createBillboardTextureCamera(mTransform, cameraForwardDir, mImageSize.x(), mImageSize.y(), fitMode);
		viewer->getCamera()->addChild(camera.camera);

		viewer->realize();
		viewer->frame();

		growEdges(*camera.albedoOutput);
		growEdges(*camera.normalOutput);
		normalizeNormalMap(*camera.normalOutput);

		Result result;
		result.albedoMap = camera.albedoOutput;
		result.normalMap = camera.normalOutput;

		return result;
	}

private:
	osg::Vec2i mImageSize;
	osg::ref_ptr<osg::MatrixTransform> mTransform;
};

class AtlasBuilder
{
public:
	AtlasBuilder(const osg::Vec2i& subImageSize, const osg::Vec2i& subImageCount)
	{
		osg::Vec2i imageSize = subImageSize * subImageCount;
		mAtlas = new osg::Image;
		mAtlas->allocateImage(imageSize.x(), imageSize.y(), 1, GL_RGBA, GL_UNSIGNED_BYTE);
	}

	void setSubImage(const osg::Vec2i& index, const osg::Image& image)
	{
		int* src = (int*)image.data();
		int* dst = (int*)mAtlas->data() + size_t(index.x() * image.s()) + size_t(index.y() * image.t() * mAtlas->s());

		for (int y = 0; y < image.t(); y++)
		{
			memcpy(dst, src, image.s() * sizeof(int));
			src += image.s();
			dst += mAtlas->s();
		}
	}

	osg::ref_ptr<osg::Image> getImage() const
	{
		return mAtlas;
	}

private:
	osg::ref_ptr<osg::Image> mAtlas;
};

struct Atlases
{
	osg::ref_ptr<osg::Image> albedoMap;
	osg::ref_ptr<osg::Image> normalMap;
};

struct Atlases generateImposterAtlas(const std::vector<std::string>& modelFilenames, const osg::Vec2i& size, int yawImageCount, float pitch, BillboardTextureFitMode fitMode)
{
	// Ensure image is a power of two
	int verticalImageCount = math::nextPow2(modelFilenames.size() * size.y()) / size.y();

	AtlasBuilder albedoBuilder(size, osg::Vec2i(yawImageCount, verticalImageCount));
	AtlasBuilder normalBuilder(size, osg::Vec2i(yawImageCount, verticalImageCount));

	int modelIndex = verticalImageCount-1;
	for (const std::string& filename : modelFilenames)
	{
		for (int yawIndex = 0; yawIndex < yawImageCount; ++yawIndex)
		{
			float yaw = 2.0f * osg::PI * (float)yawIndex / (float)yawImageCount;

			ImposterGenerator generator(filename, size);
			ImposterGenerator::Result result = generator.render(yaw, pitch, fitMode);

			albedoBuilder.setSubImage(osg::Vec2i(yawIndex, modelIndex), *result.albedoMap);
			normalBuilder.setSubImage(osg::Vec2i(yawIndex, modelIndex), *result.normalMap);
		}
		--modelIndex;
	}

	Atlases atlases;
	atlases.albedoMap = albedoBuilder.getImage();
	atlases.normalMap = normalBuilder.getImage();
	return atlases;
}

TEST_CASE("Generate billboard texture")
{/*
	std::vector<std::string> filenames = {
		"spruce_2.lwo",
		"spruce_3.lwo" ,
		"spruce_4.lwo" };

	if (1)
	{
		Atlases atlases = generateImposterAtlas(filenames, osg::Vec2i(128, 256), 8, 0, BillboardTextureFitHeight);
		osgDB::writeImageFile(*atlases.albedoMap, "spruceAtlas_side_albedo.tga");
		osgDB::writeImageFile(*atlases.normalMap, "spruceAtlas_side_normal.tga");
	}
	else
	{
		Atlases atlases = generateImposterAtlas(filenames, osg::Vec2i(128, 128), 1, math::halfPiF(), BillboardTextureFitWidth);
		osgDB::writeImageFile(*atlases.albedoMap, "spruceAtlas_top_albedo.tga");
		osgDB::writeImageFile(*atlases.normalMap, "spruceAtlas_top_normal.tga");
	}
	*/
}
