/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "LakesBatch.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgGeometryHelpers.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include <SkyboltVis/VisibilityCategory.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>

#include "SkyboltVis/earcutOsg.h"

float lakeHeightAboveTerrain = 0.01f;

//#define WIREFRAME
#ifdef WIREFRAME
#include <osg/PolygonMode>
#endif

namespace skybolt {
namespace vis {

void createLake(const Lake& lake, osg::Vec3Array* posBuffer, osg::Vec3Array* normalBuffer, osg::Vec2Array* uvBuffer, osg::UIntArray* indexBuffer)
{
	assert(lake.points.size() >= 3);
	size_t indexoffset = posBuffer->size();

	std::vector<osg::Vec3> points = lake.points;
	std::vector<std::vector<osg::Vec3f>> polygon = { points };

	std::vector<int> indices = mapbox::earcut<int>(polygon);

	for (int i = 0; i < points.size(); ++i)
	{
		posBuffer->push_back(points[i] - osg::Vec3(0,0, lakeHeightAboveTerrain));
		normalBuffer->push_back(osg::Vec3(0, 0, -1));
		uvBuffer->push_back(osg::Vec2f(i, i)); // TODO: remove UVs. They are unused.
	}

	for (int i = 0; i < indices.size(); i += 3)
	{
		indexBuffer->push_back(indexoffset + indices[i]);
		indexBuffer->push_back(indexoffset + indices[i+2]); // reverse winding
		indexBuffer->push_back(indexoffset + indices[i+1]);
	}

//#define EXPORT_OBJ
#ifdef EXPORT_OBJ
	osg::Vec2f start = points[0];
	//if (start.length() < 100000)
	{
		static int counter = 0;
		std::ofstream f("Lakes/export" + std::to_string(++counter) + ".obj");


		for (const osg::Vec2& p : points)
		{
			f << "v " << p.x() - start.x() << " " << p.y() - start.y() << " " << 0 << std::endl;
		}

		for (int i = 0; i < indices.size(); i += 3)
		{
			f << "f " << (indices[i] + 1) << " " << (indices[i + 1] + 1) << " " << (indices[i + 2] + 1) << std::endl;
		}

		f.close();
	}

#endif
}

osg::Geode* createLakes(const Lakes& lakes)
{
	osg::Vec3Array* posBuffer = new osg::Vec3Array();
	osg::Vec3Array* normalBuffer = new osg::Vec3Array();
	osg::Vec2Array* uvBuffer = new osg::Vec2Array();
	osg::UIntArray* indexBuffer = new osg::UIntArray();

	for (size_t i = 0; i < lakes.size(); ++i)
	{
		createLake(lakes[i], posBuffer, normalBuffer, uvBuffer, indexBuffer);
	}

	osg::Geode* geode = new osg::Geode();
	osg::Geometry* geometry = new osg::Geometry();

	geometry->setVertexArray(posBuffer);
	geometry->setNormalArray(normalBuffer);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	geometry->setTexCoordArray(0, uvBuffer);
	configureDrawable(*geometry);

	geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, indexBuffer->size(), (GLuint*)indexBuffer->getDataPointer()));

	geometry->setCullingActive(false); // TODO: set bounds and enable culling
	geode->addDrawable(geometry);
	return geode;
}


osg::ref_ptr<osg::StateSet> createStateSet(const osg::ref_ptr<osg::Program>& program, const LakesBatch::Uniforms& uniforms)
{
	osg::ref_ptr<osg::StateSet> ss(new osg::StateSet);
	ss->setAttributeAndModes(program, osg::StateAttribute::ON);
	ss->addUniform(uniforms.modelMatrix);

	ss->addUniform(new osg::Uniform("depthOffset", -0.0005f));
	ss->setDefine("ENABLE_DEPTH_OFFSET");
	ss->setDefine("ACCURATE_LOG_Z"); // enabled because geometry is sparsely tessellated

#ifdef WIREFRAME
	osg::PolygonMode * polygonMode = new osg::PolygonMode;
	polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

	ss->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
#endif

	return ss;
}

LakesBatch::LakesBatch(const Lakes& lakes, const LakesConfig& config)
{
	mUniforms.modelMatrix = new osg::Uniform("modelMatrix", osg::Matrixf());

	osg::Geode* geode = createLakes(lakes);
	geode->setStateSet(createStateSet(config.program, mUniforms));

	mTransform->setStateSet(config.waterStateSet);
	mTransform->setNodeMask(~vis::VisibilityCategory::shadowCaster);
	mTransform->addChild(geode);
}

void LakesBatch::updatePreRender(const RenderContext& context)
{
	mUniforms.modelMatrix->set(mTransform->getWorldMatrices().front());
}

} // namespace vis
} // namespace skybolt
