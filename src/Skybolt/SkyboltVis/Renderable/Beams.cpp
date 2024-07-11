/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Beams.h"
#include "SkyboltVis/OsgGeometryHelpers.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;
using namespace skybolt::vis;

Beams::Batch::Batch(const osg::ref_ptr<osg::Texture2D>& texture)
{
	drawArrays = new osg::DrawArrays();
	vertices = new osg::Vec3Array();
	normals = new osg::Vec3Array();
	uvs = new osg::Vec4Array();

	geometry = new osg::Geometry();
	geometry->addPrimitiveSet(drawArrays);
	configureDrawable(*geometry);

	auto stateSet = geometry->getOrCreateStateSet();
	int unit = 0;
	stateSet->setTextureAttributeAndModes(unit, texture);
	stateSet->addUniform(createUniformSampler2d("albedoSampler", unit++));
}

void Beams::Batch::setBeams(const std::vector<BeamParams>& beams, const BeamsConfig::GeometricParams& geometricParams)
{
	constexpr int verticesPerBeam = 8;

	vertices->clear();	
	vertices->reserve(beams.size() * verticesPerBeam);

	normals->clear();	
	normals->reserve(beams.size() * verticesPerBeam);

	uvs->clear();
	uvs->reserve(beams.size() * verticesPerBeam);

	osg::BoundingBox bounds;

	for (const auto& beam : beams)
	{
		float radius = beam.radius;
		osg::Vec3f tangent, bitangent;
		math::getOrthonormalBasis(beam.relDirection, tangent, bitangent);
		float baseRadius = beam.radius * std::sqrt(2) * geometricParams.baseRadiusMultiplier;
		osg::Vec3f extrusionStartPoint = beam.relPosition + beam.relDirection * beam.length * geometricParams.extrusionOffsetFraction;
		osg::Vec3f extrusionEndPoint = extrusionStartPoint + beam.relDirection * beam.length;

		// quad at the base of the beam
		vertices->push_back(beam.relPosition - tangent * baseRadius);
		vertices->push_back(beam.relPosition + bitangent * baseRadius);
		vertices->push_back(beam.relPosition + tangent * baseRadius);
		vertices->push_back(beam.relPosition - bitangent * baseRadius);
		float displacement = 0.f; // zero vertex displacement because the vertex is already at the correct radius;
		uvs->push_back(osg::Vec4f(geometricParams.basePartBounds.minimum.x(), geometricParams.basePartBounds.minimum.y(), displacement, beam.alpha));
		uvs->push_back(osg::Vec4f(geometricParams.basePartBounds.minimum.x(), geometricParams.basePartBounds.maximum.y(), displacement, beam.alpha));
		uvs->push_back(osg::Vec4f(geometricParams.basePartBounds.maximum.x(), geometricParams.basePartBounds.maximum.y(), displacement, beam.alpha));
		uvs->push_back(osg::Vec4f(geometricParams.basePartBounds.maximum.x(), geometricParams.basePartBounds.minimum.y(), displacement, beam.alpha));

		// quad that runs the length of the beam
		vertices->push_back(extrusionStartPoint);
		vertices->push_back(extrusionEndPoint);
		vertices->push_back(extrusionEndPoint);
		vertices->push_back(extrusionStartPoint);
		uvs->push_back(osg::Vec4f(geometricParams.extrusionPartBounds.minimum.x(), geometricParams.extrusionPartBounds.minimum.y(), -beam.radius, beam.alpha)); // third component is shader vertex displacement distance
		uvs->push_back(osg::Vec4f(geometricParams.extrusionPartBounds.minimum.x(), geometricParams.extrusionPartBounds.maximum.y(), -beam.radius, beam.alpha));
		uvs->push_back(osg::Vec4f(geometricParams.extrusionPartBounds.maximum.x(), geometricParams.extrusionPartBounds.maximum.y(), beam.radius, beam.alpha));
		uvs->push_back(osg::Vec4f(geometricParams.extrusionPartBounds.maximum.x(), geometricParams.extrusionPartBounds.minimum.y(), beam.radius, beam.alpha));

		for (int i = 0; i < verticesPerBeam; ++i)
		{
			normals->push_back(beam.relDirection);
		}

		// Add beam to bounding box
		bounds.expandBy(beam.relPosition - tangent - bitangent);
		bounds.expandBy(beam.relPosition - tangent + bitangent);
		bounds.expandBy(beam.relPosition + tangent - bitangent);
		bounds.expandBy(beam.relPosition + tangent + bitangent);
		bounds.expandBy(extrusionEndPoint - tangent - bitangent);
		bounds.expandBy(extrusionEndPoint - tangent + bitangent);
		bounds.expandBy(extrusionEndPoint + tangent - bitangent);
		bounds.expandBy(extrusionEndPoint + tangent + bitangent);
	}

	geometry->setVertexArray(vertices);
	geometry->setNormalArray(normals);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setTexCoordArray(0, uvs);
	geometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(bounds));
	drawArrays->set(osg::PrimitiveSet::QUADS, 0, vertices->size());
}

Beams::Beams(const BeamsConfig& config) :
	Model(ModelConfig::ofNode(osg::ref_ptr<osg::Group>(new osg::Group))),
	mGeometricParams(config.geometricParams)
{
	auto stateSet = mNode->getOrCreateStateSet();
	stateSet->setAttribute(config.program);
	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	makeStateSetTransparent(*stateSet, TransparencyMode::Classic);

	mTransform->addChild(mNode);
}

void Beams::setBeams(const std::vector<BeamParams>& beams)
{
	if (mBeams.empty() && beams.empty())
	{
		return;
	}

	mBeams = beams;

	// Removed old batches which are no longer present
	std::set<osg::ref_ptr<osg::Texture2D>> unusedTextures;
	for (const auto& [texture, batch] : mBatches)
	{
		unusedTextures.insert(texture);
	}

	std::map<osg::ref_ptr<osg::Texture2D>, std::vector<BeamParams>> textureBeamsMap;
	for (const auto& beam : beams)
	{
		textureBeamsMap[beam.texture].push_back(beam);
		unusedTextures.erase(beam.texture);
	}

	for (const auto& texture : unusedTextures)
	{
		auto i = mBatches.find(texture);
		if (i != mBatches.end())
		{
			static_cast<osg::Group*>(mNode)->removeChild(i->second.geometry);
			mBatches.erase(i);
		}
	}

	// Add/update current batches
	for (const auto& [texture, beamsForTexture] : textureBeamsMap)
	{
		if (mBatches.find(texture) == mBatches.end())
		{
			mBatches[texture] = Batch(texture);
			static_cast<osg::Group*>(mNode)->addChild(mBatches[texture].geometry);
		}

		Batch& batch = mBatches.find(texture)->second;
		batch.setBeams(beamsForTexture, mGeometricParams);
	}
}
