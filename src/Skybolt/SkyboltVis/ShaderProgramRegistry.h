/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Program>

namespace skybolt {
namespace vis {

struct ShaderPrograms
{
	osg::ref_ptr<osg::Program> cloud;
	osg::ref_ptr<osg::Program> compositeClouds;
	osg::ref_ptr<osg::Program> compositeFinal;
	osg::ref_ptr<osg::Program> glass;
	osg::ref_ptr<osg::Program> model;
	osg::ref_ptr<osg::Program> modelText;
	osg::ref_ptr<osg::Program> heightToNormal;
	osg::ref_ptr<osg::Program> vectorDisplacementToNormal;
	osg::ref_ptr<osg::Program> waveFoamMaskGenerator;
	osg::ref_ptr<osg::Program> ocean;
	osg::ref_ptr<osg::Program> shadowCaster;
	osg::ref_ptr<osg::Program> sky;
	osg::ref_ptr<osg::Program> skyToEnvironmentMap;
	osg::ref_ptr<osg::Program> starfield;
	osg::ref_ptr<osg::Program> terrainFlatTile;
	osg::ref_ptr<osg::Program> terrainPlanetTile;
	osg::ref_ptr<osg::Program> treeSideBillboard;
	osg::ref_ptr<osg::Program> treeTopBillboard;
	osg::ref_ptr<osg::Program> lake;
	osg::ref_ptr<osg::Program> planet;
	osg::ref_ptr<osg::Program> sun;
	osg::ref_ptr<osg::Program> moon;
	osg::ref_ptr<osg::Program> unlitColored;
	osg::ref_ptr<osg::Program> volumeClouds;
	osg::ref_ptr<osg::Program> building;
	osg::ref_ptr<osg::Program> hudText;
	osg::ref_ptr<osg::Program> hudGeometry;
	osg::ref_ptr<osg::Program> hudTexture3d;
};

ShaderPrograms createShaderPrograms();

} // namespace vis
} // namespace skybolt
