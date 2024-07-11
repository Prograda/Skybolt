/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Program>

namespace skybolt {
namespace vis {

using NamedProgramsMap = std::map<std::string, osg::ref_ptr<osg::Program>>;

class ShaderPrograms
{
public:
	ShaderPrograms() {};
	ShaderPrograms(const NamedProgramsMap& programs) : mPrograms(programs) {};

	//! Throws exception if program not found
	const osg::ref_ptr<osg::Program>& getRequiredProgram(const std::string& name) const;

	const NamedProgramsMap& getPrograms() const { return mPrograms; }

private:
	NamedProgramsMap mPrograms;
};

using ShaderProgramSourceFiles = std::map<osg::Shader::Type, std::string>; //!< Source code files for a program
osg::ref_ptr<osg::Program> createProgram(const std::string& name, const ShaderProgramSourceFiles& files);

ShaderProgramSourceFiles getShaderSource_terrainFlatTile();

ShaderPrograms createShaderPrograms();

} // namespace vis
} // namespace skybolt
