/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ReadJsonFile.h"
#include "SkyboltCommon/Exception.h"
#include <filesystem>
#include <fstream>
#include <istream>

namespace skybolt {

nlohmann::json readJsonFile(const std::string& filename)
{
	std::filesystem::path p(filename);
	if (!std::filesystem::exists(p))
	{
		throw Exception("File does not exist: '" + filename + "'");
	}

	std::ifstream f(filename);
	if (!f.is_open())
	{
		throw Exception("Could not open file: '" + filename + "'");
	}

	nlohmann::json json;
	try
	{
		f >> json;
	}
	catch (const std::exception& e)
	{
		throw  Exception("Error parsing file: '" + filename + "':" + e.what());
	}

	f.close();
	return json;
}

} // namespace skybolt