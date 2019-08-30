/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgShaderHelpers.h"
#include <SkyboltCommon/Exception.h>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

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
 
	static const boost::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
	stringstream input;
	stringstream output;
	input << source;
 
	size_t line_number = 1;
	boost::smatch matches;
 
	string line;
	while(std::getline(input,line))
	{
		if (boost::regex_search(line, matches, re))
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

osg::Shader* readShaderFile(osg::Shader::Type type, const std::string& filename)
{
	std::string source = loadFileToString(filename);
	std::string includeDirPath = boost::filesystem::path(filename).parent_path().string();

	source = preprocessIncludes(source, includeDirPath);

	osg::Shader* shader = new osg::Shader(type, source);
	shader->setName(filename);
	return shader;
}

} // namespace vis
} // namespace skybolt
