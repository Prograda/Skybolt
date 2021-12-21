/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>

#define GLM_FORCE_SWIZZLE
#include "Helpers/CaptureTexture.h"
#include <SkyboltVis/Shader/OsgShaderHelpers.h>
#include <SkyboltVis/TextureGenerator/CompositingPipelineFactory.h>
#include <SkyboltVis/Window/OffscreenViewer.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <glm/glm.hpp>

#include <osg/Texture2D>
#include <osgDB/WriteFile>

using namespace glm;
using namespace skybolt;
using namespace vis;

// Workaround for GLSL swizzle members not working on GCC due to lack of support for 'anonymous structs as union members'.
// They can be used as functions instead.
// See https://chromium.googlesource.com/external/github.com/g-truc/glm/+/HEAD/manual.md
#define rgb rgb()

#undef M_PI // Avoid redefinition warning due to M_PI define in cmath, included by catch.

#define ENABLE_SPECULAR
#include "Brdfs/PrincipledBrdfDirectionalLight.h"

TEST_CASE("Principled BRDF matches reference under directional light")
{
	// Tested against PBRT 3 Disney Principled BRDF implementation with eta = 1.44
	// Here's the PBRT scene file:
	/*
	LookAt 5 0 0  # eye
		   0 0 0  # look at point
		   0 0 1    # up vector
	Camera "orthographic" "float fov" 45

	Sampler "halton" "integer pixelsamples" 1
	Integrator "directlighting"
	Film "image" "string filename" "simple.exr"
		 "integer xresolution" [1024] "integer yresolution" [1024]

	WorldBegin

	# approximate the sun
	LightSource "distant"  "point from" [ 0 0  100 ]
	   "rgb L" [1 1 1 ]

	AttributeBegin
	  Material "disney"
		"rgb color" [1 0 0]
		"float roughness" 0.2
		"float eta" 1.44
	  Shape "sphere" "float radius" 1
	AttributeEnd
	WorldEnd
	*/

	PrincipledBrdfDirectionalLightArgs args;
	args.lightDirection = glm::vec3(0, 0, -1);
	args.normal = glm::vec3(-1, 0, 0);
	args.dotNL = glm::dot(args.lightDirection, args.normal);
	args.viewDirection = glm::vec3(-1, 0, 0);
	args.irradiance = vec3(1.0);
	args.specularF0 = vec3(0.04f); // equivalent to blender specular of 0.4 * 0.08 = 0.5. The 0.08 is a constant in the disney principled BSDF.
	args.roughness = 0.2f;

	SECTION("Reflectance is zero when lit perpendicular to normal")
	{
		PrincipledBrdfDirectionalLightResult result = evalPrincipledBrdfDirectionalLight(args);
		CHECK(result.diffuse.r == 0);
		CHECK(result.specular.r == 0);
	}

	SECTION("Reflectance matches blender reference at center of specular")
	{
		args.normal = glm::normalize(glm::vec3(-1, 0, -1));
		args.dotNL = glm::dot(args.lightDirection, args.normal);
		PrincipledBrdfDirectionalLightResult result = evalPrincipledBrdfDirectionalLight(args);
		CHECK(result.diffuse.r == Approx(0.23).margin(0.02));
		CHECK(result.specular.r == Approx(2.93).margin(0.02));
	}
}

// Test code for rendering a BRDF lighting sphere on the GPU
static osg::ref_ptr<osg::Texture2D> createTexture2D(int width, int height)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
	texture->setTextureWidth(width);
	texture->setTextureHeight(width);
	texture->setInternalFormat(GL_RGBA8);
	return texture;
}

static osg::ref_ptr<osg::Image> renderImageOnGpu(const osg::ref_ptr<osg::Program>& program, int width, int height)
{
	CompositingPipelineFactory factory;

	osg::ref_ptr<osgViewer::Viewer> viewer = createOffscreenViewer(width, height);
	auto camera = viewer->getCamera();
	osg::ref_ptr<ImageCaptureDrawCallback> callback = new ImageCaptureDrawCallback({});
	camera->setFinalDrawCallback(callback);

	auto texture = createTexture2D(width, height);

	CompositingStage stage;
	stage.outputs = {texture};
	stage.program = program;

	camera->addChild(factory.createCompositingPipeline({stage}));
	callback->setTexturesToCapture({ImageCaptureDrawCallback::CaptureItem(texture, GL_UNSIGNED_BYTE)});

	// Generate
	viewer->frame();

	// Check image has yellow square on blue background
	return callback->capturedImages.at(0);
}

static void renderBrdfSphere()
{
	const char vertexShader[] = R"(
    #version 440
    in vec4 osg_Vertex;
	in vec4 osg_MultiTexCoord0;
	out vec3 texCoord;
    void main() {
      gl_Position = osg_Vertex * vec4(2) - vec4(1);
	  texCoord = osg_MultiTexCoord0.xyz;
    })";

	const char fragmentShader[] = R"(
	#version 440
	#define ENABLE_SPECULAR
	#include "Brdfs/PrincipledBrdfDirectionalLight.h"
	in vec3 texCoord;
    out vec4 color;
    void main() {
		vec2 P = (texCoord.xy - 0.5) * 2.0;

		if (length(P) < 1.f)
		{
			vec3 N = vec3(P.xy, sqrt(1-dot(P,P)));

			PrincipledBrdfDirectionalLightEvalArgs args;
			args.lightDirection = vec3(0, 1, 0);
			args.normal = N;
			args.viewDirection = vec3(0, 0, 1);
			args.irradiance = vec3(1.0);
			args.specularF0 = vec3(0.04f); // equivalent to blender specular of 0.4 * 0.08 = 0.5. The 0.08 is a constant in the disney principled BSDF.
			args.roughness = 0.4f;

			PrincipledBrdfEvalResult result = evalPrincipledBrdfDirectionalLight(args);

			color = vec4(result.diffuse * vec3(1,0,0) + result.specular, 1);
		}
		else
		{
			color = vec4(0);
		}
    })";

	osg::ref_ptr<osg::Program> program = new osg::Program();
	program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShader));
	program->addShader(vis::readShaderFromString(osg::Shader::FRAGMENT, fragmentShader, std::string(SHADERS_SOURCE_DIR)));

	auto image = renderImageOnGpu(program, 512, 512);
	osgDB::writeImageFile(*image, "test.png");
}