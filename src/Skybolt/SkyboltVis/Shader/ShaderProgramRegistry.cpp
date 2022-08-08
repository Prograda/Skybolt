/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ShaderProgramRegistry.h"
#include "OsgShaderHelpers.h"

namespace skybolt {
namespace vis {

const osg::ref_ptr<osg::Program>& ShaderPrograms::getRequiredProgram(const std::string& name) const
{
	auto i = mPrograms.find(name);
	if (i != mPrograms.end())
	{
		return i->second;
	}
	throw std::runtime_error("Shader '" + name + "' not defined");
}

using ShaderProgramSourceFilesRegistry = std::map<std::string, ShaderProgramSourceFiles>;

ShaderProgramSourceFiles getShaderSource_terrainFlatTile()
{
	return {
		{ osg::Shader::VERTEX, "Shaders/TessDisplacement.vert" },
		{ osg::Shader::TESSCONTROL, "Shaders/TessDisplacement.tctrl" },
		{ osg::Shader::TESSEVALUATION, "Shaders/TessDisplacement.teval" },
		{ osg::Shader::FRAGMENT, "Shaders/TessDisplacement.frag" }
	};
}

static ShaderProgramSourceFilesRegistry createShaderProgramSourceFilesRegistry()
{
	return {
		{ "beams", {
			{ osg::Shader::VERTEX, "Shaders/Beams.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Beams.frag" }
		}},
		{ "cloud", {
			{ osg::Shader::VERTEX, "Shaders/BillboardCloud.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/BillboardCloud.frag" }
		}},
		{ "cloudsTemporalUpscaling", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/CloudsTemporalUpscaling.frag" }
		}},
		{ "compositeClouds", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/CompositeClouds.frag" }
		}},
		{ "compositeFinal", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/CompositeFinal.frag" }
		}},
		{ "glass", {
			{ osg::Shader::VERTEX, "Shaders/SimpleTextured.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Glass.frag" }
		}},
		{ "model", {
			{ osg::Shader::VERTEX, "Shaders/SimpleTextured.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/PrincipledBrdf.frag" }
		}},
		{ "modelText", {
			{ osg::Shader::VERTEX, "Shaders/SimpleTextured.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/SimpleTexturedLambertText.frag" }
		}},
		{ "road", {
			{ osg::Shader::VERTEX, "Shaders/SimpleTextured.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Road.frag" }
		}},
		{ "heightToNormal", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/HeightToNormalConverter.frag" }
		}},
		{ "vectorDisplacementToNormal", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/VectorDisplacementToNormalConverter.frag" }
		}},
		{ "waveFoamMaskGenerator", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/WaveFoamMaskGenerator.frag" }
		}},
		{ "ocean", {
			{ osg::Shader::VERTEX, "Shaders/OceanProjected.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Ocean.frag" }
		}},
		{ "shadowCaster", {
			{ osg::Shader::VERTEX, "Shaders/Shadows/SimpleShadowCaster.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Shadows/SimpleShadowCaster.frag" }
		}},
		{ "sky", {
			{ osg::Shader::VERTEX, "Shaders/Sky.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Sky.frag" }
		}},
		{ "skyToEnvironmentMap", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/SkyToEnvironmentMap.frag" }
		}},
		{ "starfield", {
			{ osg::Shader::VERTEX, "Shaders/Starfield.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Starfield.frag" }
		}},
		{ "terrainFlatTile", getShaderSource_terrainFlatTile() },
		{ "terrainPlanetTile", {
			{ osg::Shader::VERTEX, "Shaders/TessDisplacementPlanet.vert" },
			{ osg::Shader::TESSCONTROL, "Shaders/TessDisplacement.tctrl" },
			{ osg::Shader::TESSEVALUATION, "Shaders/TessDisplacement.teval" },
			{ osg::Shader::FRAGMENT, "Shaders/TessDisplacement.frag" }
		}},
		{ "treeSideBillboard", {
			{ osg::Shader::VERTEX, "Shaders/TreeSideBillboard.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/TreeBillboard.frag" }
		}},
		{ "treeTopBillboard", {
			{ osg::Shader::VERTEX, "Shaders/TreeTopBillboard.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/TreeBillboard.frag" }
		}},
		{ "lake", {
			{ osg::Shader::VERTEX, "Shaders/OceanStatic.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Ocean.frag" }
		}},
		{ "particles", {
			{ osg::Shader::VERTEX, "Shaders/Particles.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Particles.frag" }
		}},
		{ "planet", {
			{ osg::Shader::VERTEX, "Shaders/Planet.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Planet.frag" }
		}},
		{ "sun", {
			{ osg::Shader::VERTEX, "Shaders/CelestialBillboard.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Sun.frag" }
		}},
		{ "moon", {
			{ osg::Shader::VERTEX, "Shaders/CelestialBillboard.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Moon.frag" }
		}},
		{ "unlitColored", {
			{ osg::Shader::VERTEX, "Shaders/SimpleColor.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/SimpleColor.frag" }
		}},
		{"unlitTextured", {
			{ osg::Shader::VERTEX, "Shaders/SimpleTextured.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/SimpleTextured.frag" }
		}},
		{ "volumeClouds", {
			{ osg::Shader::VERTEX, "Shaders/VolumeClouds.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/VolumeClouds.frag" }
		}},
		{ "building", {
			{ osg::Shader::VERTEX, "Shaders/Building.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/Building.frag" }
		}},
		{"hudText", {
			{ osg::Shader::VERTEX, "Shaders/SimpleColorFixedScreenSize.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/HudText.frag" }
		}},
		{"hudGeometry", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/HudTexture.frag" }
		}},
		{"hudTexture3d", {
			{ osg::Shader::VERTEX, "Shaders/ScreenQuad.vert" },
			{ osg::Shader::FRAGMENT, "Shaders/HudTexture3d.frag" }
		}},
	};
}

osg::ref_ptr<osg::Program> createProgram(const std::string& name, const ShaderProgramSourceFiles& files)
{
	osg::ref_ptr<osg::Program> p = new osg::Program();
	for (auto file : files)
	{
		p->addShader(vis::readShaderFile(file.first, file.second));
	}
	return p;
}

ShaderPrograms createShaderPrograms()
{
	std::map<std::string, osg::ref_ptr<osg::Program>> programs;

	ShaderProgramSourceFilesRegistry registry = createShaderProgramSourceFilesRegistry();

	for (const auto& i : registry)
	{
		osg::ref_ptr<osg::Program> p = new osg::Program();
		for (auto shader : i.second)
		{
			p->addShader(vis::readShaderFile(shader.first, shader.second));
		}
		programs[i.first] = p;
	}

	return ShaderPrograms(programs);
}

} // namespace vis
} // namespace skybolt
