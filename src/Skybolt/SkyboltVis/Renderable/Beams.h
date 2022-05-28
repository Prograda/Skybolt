/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/Renderable/Model/Model.h"
#include "SkyboltVis/OsgBox2.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <osg/Geometry>
#include <osg/Program>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct BeamsConfig
{
	osg::ref_ptr<osg::Program> program;

	struct GeometricParams
	{
		Box2f extrusionPartBounds; //!< region of the texture map representing the extruded part of the beam
		Box2f basePartBounds; //!< region of the texture map representing the circular base of the beam at the beam's origin
		float extrusionOffsetFraction = 0; //!< How far the extrusion begins from the origin as a fraction of beam length
		float baseRadiusMultiplier = 1; //!< How large the beam's circular base should be relative to the beam radius
	};

	GeometricParams geometricParams;
};

//! Effect consisting of multiple cylindrical beams
class Beams : public Model
{
public:
	Beams(const BeamsConfig& config);
	~Beams() override = default;

	struct BeamParams
	{
		osg::ref_ptr<osg::Texture2D> texture;
		osg::Vec3f relPosition; //!< position relative to node
		osg::Vec3f relDirection; //!< direction relative to node
		float length;
		float radius;
		float alpha;
	};

	void setBeams(const std::vector<BeamParams>& beams);
	const std::vector<BeamParams>& getBeams() const { return mBeams; }

private:
	BeamsConfig::GeometricParams mGeometricParams;

	std::vector<BeamParams> mBeams;

	struct Batch
	{
		Batch(const osg::ref_ptr<osg::Texture2D>& texture);
		Batch() {}

		void setBeams(const std::vector<BeamParams>& beams, const BeamsConfig::GeometricParams& geometricParams);

		osg::ref_ptr<osg::Geometry> geometry;
		osg::ref_ptr<osg::DrawArrays> drawArrays;
		osg::ref_ptr<osg::Vec3Array> vertices;
		osg::ref_ptr<osg::Vec3Array> normals; //!< The normals store the direction of the beam
		osg::ref_ptr<osg::Vec4Array> uvs; //!< [u, v, displacementDistance, alpha]
	};

	std::map<osg::ref_ptr<osg::Texture2D>, Batch> mBatches;
};

} // namespace vis
} // namespace skybolt
