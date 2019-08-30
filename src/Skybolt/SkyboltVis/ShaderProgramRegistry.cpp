/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ShaderProgramRegistry.h"
#include "OsgShaderHelpers.h"

namespace skybolt {
namespace vis {

ShaderPrograms createShaderPrograms()
{
	ShaderPrograms p;

	p.cloud = new osg::Program();
	p.cloud->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/BillboardCloud.vert"));
	p.cloud->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/BillboardCloud.frag"));

	p.glass = new osg::Program();
	p.glass->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/SimpleTextured.vert"));
	p.glass->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Glass.frag"));

	p.compositeClouds = new osg::Program();
	p.compositeClouds->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.compositeClouds->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/CompositeClouds.frag"));

	p.compositeFinal = new osg::Program();
	p.compositeFinal->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.compositeFinal->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/CompositeFinal.frag"));

	p.model = new osg::Program();
	p.model->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/SimpleTextured.vert"));
	p.model->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/SimpleTexturedLambert.frag"));

	p.modelText = new osg::Program();
	p.modelText->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/SimpleTextured.vert"));
	p.modelText->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/SimpleTexturedLambertText.frag"));

	p.heightToNormal = new osg::Program();
	p.heightToNormal->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.heightToNormal->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/HeightToNormalConverter.frag"));

	p.vectorDisplacementToNormal = new osg::Program();
	p.vectorDisplacementToNormal->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.vectorDisplacementToNormal->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/VectorDisplacementToNormalConverter.frag"));

	p.waveFoamMaskGenerator = new osg::Program();
	p.waveFoamMaskGenerator->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.waveFoamMaskGenerator->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/WaveFoamMaskGenerator.frag"));

	p.ocean = new osg::Program();
	p.ocean->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/OceanProjected.vert"));
	p.ocean->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Ocean.frag"));

	p.shadowCaster = new osg::Program();
	p.shadowCaster->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/Shadows/SimpleShadowCaster.vert"));
	p.shadowCaster->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Shadows/SimpleShadowCaster.frag"));

	p.sky = new osg::Program();
	p.sky->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/Sky.vert"));
	p.sky->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Sky.frag"));

	p.skyToEnvironmentMap = new osg::Program();
	p.skyToEnvironmentMap->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.skyToEnvironmentMap->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/SkyToEnvironmentMap.frag"));

	p.starfield = new osg::Program();
	p.starfield->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/Starfield.vert"));
	p.starfield->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Starfield.frag"));

	p.terrainFlatTile = new osg::Program();
	p.terrainFlatTile->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/TessDisplacement.vert"));
	p.terrainFlatTile->addShader(readShaderFile(osg::Shader::TESSCONTROL, "Shaders/TessDisplacement.tctrl"));
	p.terrainFlatTile->addShader(readShaderFile(osg::Shader::TESSEVALUATION, "Shaders/TessDisplacement.teval"));
	p.terrainFlatTile->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/TessDisplacement.frag"));

	p.terrainPlanetTile = new osg::Program();
	p.terrainPlanetTile->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/TessDisplacementPlanet.vert"));
	p.terrainPlanetTile->addShader(readShaderFile(osg::Shader::TESSCONTROL, "Shaders/TessDisplacement.tctrl"));
	p.terrainPlanetTile->addShader(readShaderFile(osg::Shader::TESSEVALUATION, "Shaders/TessDisplacement.teval"));
	p.terrainPlanetTile->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/TessDisplacement.frag"));

	p.treeSideBillboard = new osg::Program();
	p.treeSideBillboard->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/TreeSideBillboard.vert"));
	p.treeSideBillboard->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/TreeBillboard.frag"));

	p.treeTopBillboard = new osg::Program();
	p.treeTopBillboard->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/TreeTopBillboard.vert"));
	p.treeTopBillboard->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/TreeBillboard.frag"));

	p.lake = new osg::Program();
	p.lake->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/OceanStatic.vert"));
	p.lake->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Ocean.frag"));

	p.planet = new osg::Program();
	p.planet->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/Planet.vert"));
	p.planet->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Planet.frag"));

	p.sun = new osg::Program();
	p.sun->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/CelestialBillboard.vert"));
	p.sun->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Sun.frag"));

	p.moon = new osg::Program();
	p.moon->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/CelestialBillboard.vert"));
	p.moon->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Moon.frag"));

	p.unlitColored = new osg::Program();
	p.unlitColored->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/SimpleColor.vert"));
	p.unlitColored->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/SimpleColor.frag"));

	p.volumeClouds = new osg::Program();
	p.volumeClouds->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/VolumeClouds.vert"));
	p.volumeClouds->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/VolumeClouds.frag"));

	p.building = new osg::Program();
	p.building->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/Building.vert"));
	p.building->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/Building.frag"));

	p.hudText = new osg::Program();
	p.hudText->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/SimpleColorFixedScreenSize.vert"));
	p.hudText->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/HudText.frag"));

	p.hudGeometry = new osg::Program();
	p.hudGeometry->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.hudGeometry->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/HudTexture.frag"));

	p.hudTexture3d = new osg::Program();
	p.hudTexture3d->addShader(readShaderFile(osg::Shader::VERTEX, "Shaders/ScreenQuad.vert"));
	p.hudTexture3d->addShader(readShaderFile(osg::Shader::FRAGMENT, "Shaders/HudTexture3d.frag"));

	return p;
}

} // namespace vis
} // namespace skybolt
