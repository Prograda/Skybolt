/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include "Helpers/CaptureTexture.h"
#include "Helpers/CheckingHelpers.h"

#include <SkyboltVis/Camera.h>
#include <SkyboltVis/OsgTextureHelpers.h>
#include <SkyboltVis/RenderContext.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/Renderable/Planet/Terrain.h>
#include <SkyboltVis/Renderable/Planet/Tile/HeightMapElevationBounds.h>
#include <SkyboltVis/Renderable/Planet/Tile/HeightMapElevationRerange.h>
#include <SkyboltVis/RenderOperation/ViewportStateSet.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <SkyboltVis/Window/OffscreenViewer.h>

#include <osg/Camera>
#include <osg/RenderInfo>
#include <osg/State>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/Registry>
#include <osgDB/WriteFile>

using namespace skybolt;
using namespace vis;

template <typename T>
static osg::ref_ptr<osg::Image> createTestImage(GLint internalFormat, int width, int height, const std::vector<T>& data)
{
	assert(data.size() == width * height);

	GLenum pixelFormat, dataType;
	switch(internalFormat)
	{
	case GL_R16:
		pixelFormat = GL_LUMINANCE;
		dataType = GL_UNSIGNED_SHORT;
		break;
	case GL_RGBA8:
		pixelFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	default:
		throw std::runtime_error("Unhandled internal texture format");
	}

	assert(data.size() == 16);
	osg::Image* image = new osg::Image();
	image->allocateImage(4, 4, 1, pixelFormat, dataType);
	image->setInternalTextureFormat(internalFormat);
	memcpy((T*)image->getDataPointer(), data.data(), data.size() * sizeof(T));
	return image;
}

static osg::ref_ptr<osg::Image> createHeightMap(int width, int height, const std::vector<std::uint16_t>& data)
{
	assert(data.size() == width * height);
	osg::ref_ptr<osg::Image> image = createTestImage(GL_R16, width, height, data);
	setHeightMapElevationRerange(*image, HeightMapElevationRerange(1.0f, 0.0f));
	setHeightMapElevationBounds(*image, HeightMapElevationRerange(0.0f, 30.0f));
	return image;
}

static osg::ref_ptr<osg::Image> createRedGreenXYMap(int width, int height)
{
	std::vector<osg::Vec4ub> albedoMapData(width * height);
	for (int i = 0; i < albedoMapData.size(); ++i)
	{
		albedoMapData[i] = osg::Vec4ub((i % width) * 255 / (width-1), (i / height) * 255 / (height-1), 0, 0);
	}
	return createTestImage(GL_RGBA8, width, height, albedoMapData);
}

static osg::ref_ptr<osg::Camera> createRenderToImageCamera(const osg::ref_ptr<osg::Image>& image)
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setViewport(0, 0, image->s(), image->t());
	camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
	camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0), image);
	return camera;
}

static TerrainConfig createTestTerrainConfig(const osg::ref_ptr<osg::Image>& heightMap, const osg::ref_ptr<osg::Image>& overallAlbedoMap)
{
	osgDB::Registry::instance()->getDataFilePathList().push_back(std::string(ASSERTS_CORE_DIR));

	auto tile = std::make_shared<TerrainConfig::FlatTile>();
	tile->size = osg::Vec2f(100, 100);
	TerrainConfig c;
	c.tile = tile;
	c.heightMap = new osg::Texture2D(heightMap);
	c.normalMap = new osg::Texture2D(createTestImage(GL_RGBA8, 4, 4, std::vector<osg::Vec4ub>(16, osg::Vec4ub(128, 128, 255, 0))));
	c.landMask = new osg::Texture2D(createTestImage(GL_RGBA8, 4, 4, std::vector<osg::Vec4ub>(16, osg::Vec4ub(1, 1, 1, 1))));
	c.overallAlbedoMap = new osg::Texture2D(overallAlbedoMap);
	c.program = createProgram("terrainFlatTile", getShaderSource_terrainFlatTile());
	return c;
}

static void updateScene(Scene& scene, const Camera& camera, const osg::Image& image)
{
	scene.updatePreRender([&] {
		CameraRenderContext c(camera);
		c.lightDirection = osg::Vec3(0,0,-1);
		c.targetDimensions = osg::Vec2i(image.s(), image.t());
		return c;
	}());
}

TEST_CASE("Terrain heightfield renderable")
{
	// Create output image
	int width = 128;
	osg::ref_ptr<osgViewer::Viewer> viewer = createOffscreenViewer(width, width);

	osg::Image* image = new osg::Image();
	image->allocateImage(width, width, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	image->setInternalTextureFormat(GL_RGBA8);
	osg::ref_ptr<osg::Camera> camera = createRenderToImageCamera(image);
	viewer->getCamera()->addChild(camera);
	viewer->getCamera()->setClearColor(osg::Vec4(0, 0, 0, 0));

	Camera sceneCamera(/* aspectRatio */ 1.0);

	osg::ref_ptr<ViewportStateSet> stateSet = new ViewportStateSet();
	camera->setStateSet(stateSet);
	Scene scene(stateSet);
	scene.setAmbientLightColor(osg::Vec3f(1.f, 1.f, 1.f));

	float epsilon = 2.f / 256.f;

	SECTION("Terrain colored by albedo map with CLAMP_EDGE texture alignment")
	{
		Terrain terrain(createTestTerrainConfig(createHeightMap(4, 4, std::vector<std::uint16_t>(16, 0)), createRedGreenXYMap(4, 4)));
		camera->addChild(terrain._getNode());

		// Update scene
		sceneCamera.setPosition(osg::Vec3d(0, 0, -190)); // set camera height to make terrain full frame
		osg::Quat quat;
		quat.makeRotate(osg::PI_2f, osg::Vec3f(0,-1,0));
		sceneCamera.setOrientation(quat);
		sceneCamera.updateOsgCameraGeometry(*camera);
		updateScene(scene, sceneCamera, *image);

		stateSet->update(sceneCamera);

		// Render
		viewer->frame();

		// uvs in range [0, 1/(2*dim)] should be 0 red, 0 green
		CHECK(almostEqual(osg::Vec4(0, 0, 0, 1), image->getColor(osg::Vec2(0.0, 0.0)), epsilon));
		CHECK(almostEqual(osg::Vec4(0, 0, 0, 1), image->getColor(osg::Vec2(0.11, 0.11)), epsilon));
		CHECK(!almostEqual(osg::Vec4(0, 0, 0, 1), image->getColor(osg::Vec2(0.13, 0.13)), epsilon));

		// uvs in range [1 - 1/(2*dim), 1] should be 1 red, 1 green
		CHECK(almostEqual(osg::Vec4(1, 1, 0, 1), image->getColor(osg::Vec2(1.0, 1.0)), epsilon));
		CHECK(almostEqual(osg::Vec4(1, 1, 0, 1), image->getColor(osg::Vec2(0.89, 0.89)), epsilon));
		CHECK(!almostEqual(osg::Vec4(1, 1, 0, 1), image->getColor(osg::Vec2(1.87, 0.87)), epsilon));

		// uv at [0, 0.5] should be 0 red, 0.5 green
		CHECK(almostEqual(osg::Vec4(0, 0.5, 0, 1), image->getColor(osg::Vec2(0, 0.5)), epsilon));
	}

	SECTION("Elevation from height map with CLAMP_EDGE texture alignment")
	{
		osg::ref_ptr<osg::Image> heightMap = createHeightMap(4, 4, std::vector<std::uint16_t>({
			0, 0, 0, 0,
			0, 25, 25, 12,
			0, 25, 25, 12,
			0, 50, 50, 25
		}));

		Terrain terrain(createTestTerrainConfig(heightMap, createRedGreenXYMap(4, 4)));
		camera->addChild(terrain._getNode());

		// Update scene
		sceneCamera.setPosition(osg::Vec3d(-5000, 0, 0)); // view full terrain along +X axis
		sceneCamera.updateOsgCameraGeometry(*camera);
		sceneCamera.setFovY(0.02); // narrow FOV to approximate orthographic projection
		updateScene(scene, sceneCamera, *image);

		stateSet->update(sceneCamera);

		// Render
		viewer->frame();

		// Center of minimum elevation should have 0 green, 0.5 red
		CHECK(almostEqual(osg::Vec4(0.5, 0, 0, 1), image->getColor(osg::Vec2(0.5, 0.51)), epsilon));

		// Center of maximum elevation should have 0 green, 0.5 red
		CHECK(almostEqual(osg::Vec4(0.5, 1, 0, 1), image->getColor(osg::Vec2(0.5, 1.0)), 0.01));

		// -Y edge should have zero height
		CHECK(almostEqual(osg::Vec4(0, 0, 0, 0), image->getColor(osg::Vec2(0.0, 0.51)), epsilon));

		 // Test that CLAMP_EDGE clamps u in range [0, 1/(2*dim)]
		CHECK(almostEqual(osg::Vec4(0, 0, 0, 0), image->getColor(osg::Vec2(0.011, 0.51)), epsilon));
		CHECK(image->getColor(osg::Vec2(0.13, 0.51)).a() > epsilon);

		 // Test that CLAMP_EDGE clamps u in range [1 - 1/(2*dim), 1]
		CHECK(image->getColor(osg::Vec2(0.85, 0.76)).a() > epsilon);
		CHECK(image->getColor(osg::Vec2(0.089, 0.76)).a() < epsilon);
	}

	SECTION("Heightmap uv scale and offset applied")
	{
		osg::ref_ptr<osg::Image> heightMap = createHeightMap(4, 4, std::vector<std::uint16_t>({
			0, 0, 0, 0,
			0, 50, 50, 0,
			0, 50, 50, 0,
			0, 0, 0, 0
		}));

		ScaleOffset scaleOffset = calcHalfTexelEdgeRemovalScaleOffset(osg::Vec2i(4, 4));

		TerrainConfig config = createTestTerrainConfig(heightMap, createRedGreenXYMap(4, 4));
		config.heightMapUvScale = math::componentWiseMultiply(config.heightMapUvScale, scaleOffset.scale);
		config.heightMapUvOffset += scaleOffset.offset;
		Terrain terrain(config);
		camera->addChild(terrain._getNode());

		// Update scene
		sceneCamera.setPosition(osg::Vec3d(-5000, 0, 0)); // view full terrain along +X axis
		sceneCamera.updateOsgCameraGeometry(*camera);
		sceneCamera.setFovY(0.02); // narrow FOV to approximate orthographic projection
		updateScene(scene, sceneCamera, *image);

		stateSet->update(sceneCamera);

		// Render
		viewer->frame();

		// Center of minimum elevation should have 0 green, 0.5 red
		CHECK(almostEqual(osg::Vec4(0.5, 0, 0, 1), image->getColor(osg::Vec2(0.5, 0.51)), epsilon));

		 // Test that CLAMP_EDGE clamps u in range [0, 1/(2*dim)]
		CHECK(image->getColor(osg::Vec2(0.0, 0.52)).a() < epsilon);
		CHECK(image->getColor(osg::Vec2(0.01, 0.52)).a() > epsilon);

		 // Test that CLAMP_EDGE clamps u in range [1 - 1/(2*dim), 1]
		CHECK(image->getColor(osg::Vec2(0.99, 0.52)).a() > epsilon);
		CHECK(image->getColor(osg::Vec2(1.0, 0.52)).a() < epsilon);
	}
}
