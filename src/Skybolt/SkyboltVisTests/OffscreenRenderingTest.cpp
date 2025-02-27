/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include "Helpers/CheckingHelpers.h"

#include <SkyboltVis/OsgGeometryFactory.h>
#include <SkyboltVis/Shader/OsgShaderHelpers.h>
#include <SkyboltVis/Window/OffscreenViewer.h>

using namespace skybolt::vis;

constexpr float epsilon = 0.01f;

static osg::ref_ptr<osg::Program> createSimpleProgram()
{
	const char vertexShader[] = R"(
    #version 440
    in vec4 osg_Vertex;
	out vec3 texCoord;
    void main() {
      gl_Position = (osg_Vertex * vec4(2) - vec4(1));
    })";

	const char fragmentShader[] = R"(
	#version 440
	in vec3 texCoord;
    out vec4 color;
    void main() {
		color = vec4(0.1, 0.2, 0.3, 1.0);
    })";

	osg::ref_ptr<osg::Program> program = new osg::Program();
	program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShader));
	program->addShader(readShaderFromString(osg::Shader::FRAGMENT, fragmentShader));
	return program;
}

TEST_CASE("Render to an offscreen image")
{
	// Create output image
	int width = 64;
	int height = 64;
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);

	// Create viewer
	osg::ref_ptr<osgViewer::Viewer> viewer = createOffscreenViewer(width, width);
	configureViewerToRenderToImage(*viewer, image);

	// Create scree quad geometry
	BoundingBox2f box(osg::Vec2(0,0), osg::Vec2(1,1));
	osg::ref_ptr<osg::Geometry> geometry = createQuad(box, QuadUpDirection::QuadUpDirectionY);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->getOrCreateStateSet()->setAttribute(createSimpleProgram());
	geode->addDrawable(geometry);

	// Add geometry to scene
	viewer->setSceneData(geode);

	// Render frame
	viewer->frame();

	// Check that the image contains the expected data
	CHECK(almostEqual(osg::Vec4(0.1, 0.2, 0.3, 1), image->getColor(0,0), epsilon));
}
