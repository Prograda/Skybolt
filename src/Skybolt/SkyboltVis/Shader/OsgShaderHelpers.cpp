/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgShaderHelpers.h"
#include <SkyboltCommon/Exception.h>

#include <filesystem>
#include <regex>

#include <osgDB/FileUtils>

#include <fstream>
#include <iostream>
#include <sstream>

using skybolt::Exception;

namespace skybolt {
namespace vis {

std::string loadFileToString(const std::string& filepath)
{
	std::string locatedFilepath = osgDB::findDataFile(filepath);

	std::string outString;
	std::ifstream stream(locatedFilepath, std::ios::in);
	if(stream.is_open())
	{
		std::string Line = "";
		while(getline(stream, Line))
			outString += "\n" + Line;
		stream.close();
	}
	else
	{
		throw Exception("Could not open file: " + filepath);
	}

	return outString;
}

static std::string preprocessIncludes(const std::string& source, const std::string& includeDirPath, int level = 0)
{
	if(level > 32)
		throw std::runtime_error("header inclusion depth limit reached, might be caused by cyclic header inclusion");
	using namespace std;
 
	static const std::regex re("^[ \t]*#[ ]*include[ ]+[\"<](.*)[\">].*");
	stringstream input;
	stringstream output;
	input << source;
 
	size_t line_number = 1;
	std::smatch matches;
 
	string line;
	while(std::getline(input,line))
	{
		if (std::regex_search(line, matches, re))
		{
			std::string includeFile = matches[1];
			std::string includeSource = loadFileToString(includeDirPath + "/" + includeFile);
			output << preprocessIncludes(includeSource, includeDirPath, level + 1) << endl;
		}
		else
		{
			output <<  line << endl;
		}
		++line_number;
	}
	return output.str();
}

osg::Shader* readShaderFromString(osg::Shader::Type type, const std::string& source, const std::optional<std::string>& includeDirPath)
{
	std::string processedSource = includeDirPath ? preprocessIncludes(source, *includeDirPath) : source;
	osg::Shader* shader = new osg::Shader(type, processedSource);
	return shader;
}

osg::Shader* readShaderFile(osg::Shader::Type type, const std::string& filename)
{
	auto resolvedFilename = osgDB::Registry::instance()->findDataFile(filename, nullptr, osgDB::CASE_SENSITIVE);
	if (resolvedFilename.empty())
	{
		throw std::runtime_error("Could not find shader file: " + filename);
	}

	std::string source = loadFileToString(resolvedFilename);
	std::string includeDirPath = std::filesystem::path(filename).parent_path().string();

	osg::Shader* shader = readShaderFromString(type, source, includeDirPath);
	shader->setFileName(filename);
	return shader;
}

} // namespace vis
} // namespace skybolt
