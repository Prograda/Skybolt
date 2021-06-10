/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BillboardTextureGenerator.h"
#include <SkyboltVis/OsgImageHelpers.h>
#include "SkyboltVis/Shader/OsgShaderHelpers.h"
#include <SkyboltCommon/Exception.h>

#include <osg/Camera>
#include <osg/ComputeBoundsVisitor>

using skybolt::Exception;

namespace skybolt {
namespace vis {

osg::StateSet* createAlbedoStateSet()
{
	osg::StateSet* ss = new osg::StateSet;

	osg::Program* program = new osg::Program();
	program->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/RenderToBillboard.vert"));
	program->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/RenderToBillboard.frag"));

	ss->setAttribute(program);

	return ss;
}

BillboardTextureCamera createBillboardTextureCamera(const osg::ref_ptr<osg::Node>& model, const osg::Vec3f& cameraForwardDir,  int width, int height, BillboardTextureFitMode fitMode)
{
	// Create the texture to render to
	osg::ref_ptr<osg::Image> albedoOutput (new osg::Image);
	albedoOutput->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);

	osg::ref_ptr<osg::Image> normalOutput (new osg::Image);
	normalOutput->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);

	// Create camera
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setViewport(0, 0, width, height);
	camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
	camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0), albedoOutput);
	camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER1), normalOutput);
	camera->addChild(model);
	camera->setClearColor(osg::Vec4(0,0,0,0));
	camera->setStateSet(createAlbedoStateSet());

	// Set camera view matrix
	osg::ComputeBoundsVisitor visitor;

	if (osg::Transform* transform = dynamic_cast<osg::Transform*>(model.get()))
		visitor.apply(*transform);
	else if (osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(model.get()))
		visitor.apply(*drawable);
	osg::BoundingBox& bounds = visitor.getBoundingBox();
	osg::Vec3f center = bounds.center();

	osg::Vec3f up(0, 0, -1);
	if (up * cameraForwardDir >= 0.9999)
	{
		up = osg::Vec3f(1, 0, 0);
	}

	camera->setViewMatrixAsLookAt(center - cameraForwardDir * bounds.radius() * 2.0f, center, up);

	// Set camera projection matrix
	float halfOrthoWidth = 0.5f * osg::Vec2f(bounds.xMax() - bounds.xMin(), bounds.yMax() - bounds.yMin()).length();
	float halfOrthoHeight = 0.5f * (bounds.zMax() - bounds.zMin());

	float imageAspectRatio = (float)width / (float)height;

	if (fitMode == BillboardTextureFitBest)
	{
		float aabbAspectRatio = halfOrthoWidth / halfOrthoHeight;
		fitMode = (imageAspectRatio > aabbAspectRatio) ? BillboardTextureFitHeight : BillboardTextureFitWidth;
	}

	if (fitMode == BillboardTextureFitHeight)
		halfOrthoWidth = halfOrthoHeight * imageAspectRatio;
	else
		halfOrthoHeight = halfOrthoWidth / imageAspectRatio;
	
	camera->setProjectionMatrixAsOrtho2D(-halfOrthoWidth, halfOrthoWidth, -halfOrthoHeight, halfOrthoHeight);

	// Render
	BillboardTextureCamera result;
	result.camera = camera;
	result.albedoOutput = albedoOutput;
	result.normalOutput = normalOutput;
	return result;
}

bool hasImmediateNeighborWithAlphaAboveThreshold(osg::Image& image, int x, int y, float alphaThreshold)
{
	int x0 = std::max(x-1, 0);
	int x1 = std::min(x+1, image.s() - 1);
	int y0 = std::max(y-1, 0);
	int y1 = std::min(y+1, image.t() - 1);

	for (int ny = y0; ny <= y1; ++ny)
	{
		for (int nx = x0; nx <= x1; ++nx)
		{
			osg::Vec4f c = image.getColor(nx, ny);
			if (c.a() >= alphaThreshold)
			{
				return true;
			}
		}
	}
	return false;
}

osg::Vec4f getAverageNeighborColor(osg::Image& image, int x, int y, float alphaThreshold)
{
	osg::Vec4f result = osg::Vec4f();
	int neighborCount = 0;

	const int filterRadius = 2;
	int x0 = std::max(x-filterRadius, 0);
	int x1 = std::min(x+filterRadius, image.s() - 1);
	int y0 = std::max(y-filterRadius, 0);
	int y1 = std::min(y+filterRadius, image.t() - 1);

	for (int ny = y0; ny <= y1; ++ny)
	{
		for (int nx = x0; nx <= x1; ++nx)
		{
			osg::Vec4f c = image.getColor(nx, ny);
			if (c.a() >= alphaThreshold)
			{
				result += c;
				++neighborCount;
			}
		}
	}
	if (neighborCount > 0)
		result = result / (float)neighborCount;
	return result;
}

void growEdges(osg::Image& image, int iterations)
{
	// Store original alpha
	std::vector<float> originalAlpha(image.s() * image.t());
	for (int y = 0; y < image.t(); ++y)
	{
		for (int x = 0; x < image.s(); ++x)
		{
			originalAlpha[x + image.s() * y] = image.getColor(x, y).a();
		}
	}

	// Grow edges
	osg::ref_ptr<osg::Image> dst = (osg::Image*)image.clone(osg::CopyOp::DEEP_COPY_ALL);
	for (int i = 0; i < iterations; ++i)
	{
		bool changed = false;
		for (int y = 0; y < image.t(); ++y)
		{
			for (int x = 0; x < image.s(); ++x)
			{
				float alphaThreshold = 0.01;
				osg::Vec4f c = image.getColor(x, y);
				if (c.a() < alphaThreshold)
				{
					if (hasImmediateNeighborWithAlphaAboveThreshold(image, x, y, alphaThreshold))
					{
						osg::Vec4f avg = getAverageNeighborColor(image, x, y, alphaThreshold);
						c.r() = avg.r();
						c.g() = avg.g();
						c.b() = avg.b();
						c.a() = 1;
						setPixelColor(*dst, x, y, c);
						changed = true;
					}
				}
			}
		}
		memcpy((void*)image.getDataPointer(), dst->getDataPointer(), image.getTotalDataSize());
		if (!changed)
			break;
	}

	// Restore original alpha
	for (int y = 0; y < image.t(); ++y)
	{
		for (int x = 0; x < image.s(); ++x)
		{
			osg::Vec4f c = image.getColor(x, y);
			c.a() = originalAlpha[x + image.s() * y];
			setPixelColor(image, x, y, c);
		}
	}
}

void normalizeNormalMap(osg::Image& image)
{
	for (int y = 0; y < image.t(); ++y)
	{
		for (int x = 0; x < image.s(); ++x)
		{
			osg::Vec4f c = image.getColor(x, y);
			osg::Vec3f normal(2.0f * c.r() - 1.0f, c.g() * 2.0f - 1.0f, c.b() * 2.0f - 1.0f);
			normal.normalize();
			c.r() = normal.x() * 0.5f + 0.5f;
			c.g() = normal.y() * 0.5f + 0.5f;
			c.b() = normal.z() * 0.5f + 0.5f;
			setPixelColor(image, x, y, c);
		}
	}
}

} // namespace vis
} // namespace skybolt
