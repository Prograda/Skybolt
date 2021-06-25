/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include "Helpers/CaptureTexture.h"
#include "Helpers/CheckingHelpers.h"

#include <SkyboltVis/TextureGenerator/CompositingPipelineFactory.h>
#include <SkyboltVis/Window/OffscreenViewer.h>

#include <osg/Camera>
#include <osg/RenderInfo>
#include <osg/State>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/Registry>
#include <osgDB/WriteFile>

#include <boost/algorithm/string/replace.hpp>

using namespace skybolt;
using namespace vis;

static std::string getVertexProgram()
{
	return R"(
#version 330 core
in vec4 osg_Vertex;
in vec4 osg_MultiTexCoord0;
out vec3 texCoord;

void main() {
	gl_Position = osg_Vertex * vec4(2) - vec4(1);
	texCoord = osg_MultiTexCoord0.xyz;
}
	)";
}

static osg::ref_ptr<osg::Program> createProgramThatDrawsYellowSquareOnBlueBackground()
{
	osg::ref_ptr<osg::Program> program = new osg::Program();
	program->addShader(new osg::Shader(osg::Shader::VERTEX, getVertexProgram()));

	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, R"(
#version 330 core

in vec3 texCoord;
out vec4 color;

void main()
{
	vec2 d = abs(texCoord.xy - vec2(0.5));
	bool background = max(d.x, d.y) > 0.25;
	color = background ? vec4(0,0,1,1) : vec4(1,1,0,1);
}
	)"));

	return program;
}

static osg::ref_ptr<osg::Program> createProgramThatDividesInputByTwo()
{
	osg::ref_ptr<osg::Program> program = new osg::Program();
	program->addShader(new osg::Shader(osg::Shader::VERTEX, getVertexProgram()));

	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, R"(
#version 330 core

in vec3 texCoord;
out vec4 color;
uniform sampler2D inputTexture;

void main()
{
	color = texture(inputTexture, texCoord.xy) / 2.0;
}
	)"));

	return program;
}

static osg::ref_ptr<osg::Program> createProgramThatDrawsConstantIntensityToTwoTargets(float value)
{
	osg::ref_ptr<osg::Program> program = new osg::Program();
	program->addShader(new osg::Shader(osg::Shader::VERTEX, getVertexProgram()));

	std::string fragment = R"(
#version 330 core

void main()
{
	gl_FragData[0] = vec4(%%%);
	gl_FragData[1] = vec4(%%%);
}
	)";
	boost::replace_all(fragment, "%%%", std::to_string(value));

	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragment));

	return program;
}

static osg::ref_ptr<osg::Program> createProgramThatDrawsConstantIntensityToLayer(float value)
{
	osg::ref_ptr<osg::Program> program = new osg::Program();
	program->addShader(new osg::Shader(osg::Shader::VERTEX, getVertexProgram()));

	program->addShader(new osg::Shader(osg::Shader::GEOMETRY, R"(
#version 330
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
uniform int layer;
void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_Layer = layer;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    gl_Layer = layer;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    gl_Layer = layer;
    EmitVertex();
    EndPrimitive();
})"));

	std::string fragment = R"(
#version 330 core

out vec4 color;

void main()
{
	color = vec4(%%%);
}
)";
	boost::replace_all(fragment, "%%%", std::to_string(value));

	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragment));

	return program;
}


constexpr int width = 8;
constexpr float epsilon = 1.0 / 256.0;

osg::ref_ptr<osg::Texture2D> createTexture2D()
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
	texture->setTextureWidth(width);
	texture->setTextureHeight(width);
	texture->setInternalFormat(GL_RGBA8);
	return texture;
}

osg::ref_ptr<osg::Texture3D> createTexture3DWithDepth(int depth)
{
	osg::ref_ptr<osg::Texture3D> texture = new osg::Texture3D();
	texture->setTextureWidth(width);
	texture->setTextureHeight(width);
	texture->setTextureDepth(depth);
	texture->setInternalFormat(GL_RGBA8);
	return texture;
}

TEST_CASE("CompositingPipeline renders expected images")
{
	CompositingPipelineFactory factory;

	osg::ref_ptr<osgViewer::Viewer> viewer = createOffscreenViewer(width, width);
	auto camera = viewer->getCamera();
	osg::ref_ptr<ImageCaptureDrawCallback> callback = new ImageCaptureDrawCallback({});
	camera->setFinalDrawCallback(callback);

	SECTION("One stage")
	{
		auto texture = createTexture2D();

		CompositingStage stage;
		stage.outputs = { texture };
		stage.program = createProgramThatDrawsYellowSquareOnBlueBackground();

		camera->addChild(factory.createCompositingPipeline({ stage }));
		callback->setTexturesToCapture({ ImageCaptureDrawCallback::CaptureItem(texture, GL_UNSIGNED_BYTE) });

		// Generate
		viewer->frame();

		// Check image has yellow square on blue background
		auto image = callback->capturedImages.at(0);

		CHECK(almostEqual(osg::Vec4(1, 1, 0, 1), image->getColor(osg::Vec2(0.5, 0.5)), epsilon));
		CHECK(almostEqual(osg::Vec4(0, 0, 1, 1), image->getColor(osg::Vec2(0, 0)), epsilon));
		CHECK(almostEqual(osg::Vec4(0, 0, 1, 1), image->getColor(osg::Vec2(1, 1)), epsilon));
	}

	SECTION("Multiple stages")
	{
		auto texture1 = createTexture2D();
		auto texture2 = createTexture2D();

		CompositingStage stage1;
		stage1.outputs = { texture1 };
		stage1.program = createProgramThatDrawsYellowSquareOnBlueBackground();

		CompositingStage stage2;
		stage2.inputs = { CompositingStage::Input("inputTexture", texture1) };
		stage2.outputs = { texture2 };
		stage2.program = createProgramThatDividesInputByTwo();

		camera->removeChildren(0, 1);
		camera->addChild(factory.createCompositingPipeline({ stage1, stage2 }));

		callback->setTexturesToCapture({ ImageCaptureDrawCallback::CaptureItem(texture2, GL_UNSIGNED_BYTE) });
		callback->capturedImages.clear();

		// Generate
		viewer->frame();

		// Check image has yellow square at half intensity
		CHECK(almostEqual(osg::Vec4(0.5, 0.5, 0.0, 0.5), callback->capturedImages.at(0)->getColor(osg::Vec2(0.5, 0.5)), epsilon));
	}

	SECTION("Additive blending on some outputs")
	{
		auto texture1 = createTexture2D();
		auto texture2 = createTexture2D();

		CompositingStage stage1;
		stage1.outputs = { texture1, texture2 };
		stage1.program = createProgramThatDrawsConstantIntensityToTwoTargets(0.5);

		CompositingStage stage2;
		stage2.outputs = { texture1, texture2 };
		stage2.additive = { true, false };
		stage2.program = createProgramThatDrawsConstantIntensityToTwoTargets(0.2);

		camera->removeChildren(0, 1);
		camera->addChild(factory.createCompositingPipeline({ stage1, stage2 }));

		callback->setTexturesToCapture({
			ImageCaptureDrawCallback::CaptureItem(texture1, GL_UNSIGNED_BYTE),
			ImageCaptureDrawCallback::CaptureItem(texture2, GL_UNSIGNED_BYTE)
		});
		callback->capturedImages.clear();

		// Generate
		viewer->frame();

		// Check images have expected intensities
		CHECK(almostEqual(osg::Vec4(0.7, 0.7, 0.7, 0.7), callback->capturedImages.at(0)->getColor(osg::Vec2(0.5, 0.5)), epsilon));
		CHECK(almostEqual(osg::Vec4(0.2, 0.2, 0.2, 0.2), callback->capturedImages.at(1)->getColor(osg::Vec2(0.5, 0.5)), epsilon));
	}

	SECTION("Writing to different layers of a 3D texture")
	{
		int depth = 2;
		auto texture = createTexture3DWithDepth(depth);

		CompositingStage stage1;
		stage1.outputs = { texture };
		stage1.program = createProgramThatDrawsConstantIntensityToLayer(0.2);
		stage1.uniforms = { new osg::Uniform("layer", 0) };

		CompositingStage stage2;
		stage2.outputs = { texture };
		stage2.program = createProgramThatDrawsConstantIntensityToLayer(0.6);
		stage2.uniforms = { new osg::Uniform("layer", 1) };

		camera->addChild(factory.createCompositingPipeline({ stage1, stage2 }));
		callback->setTexturesToCapture({ ImageCaptureDrawCallback::CaptureItem(texture, GL_UNSIGNED_BYTE) });

		// Generate
		viewer->frame();

		// Check image has two layers with expected intensities
		auto image = callback->capturedImages.at(0);

		CHECK(almostEqual(osg::Vec4(0.2, 0.2, 0.2, 0.2), image->getColor(osg::Vec3(0.5, 0.5, 0.0)), epsilon));
		CHECK(almostEqual(osg::Vec4(0.6, 0.6, 0.6, 0.6), image->getColor(osg::Vec3(0.5, 0.5, 1.0)), epsilon));
	}
}
