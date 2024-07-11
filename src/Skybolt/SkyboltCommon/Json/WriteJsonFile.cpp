/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WriteJsonFile.h"
#include <fstream>
#include <iomanip>
#include <ostream>

namespace skybolt {

void writeJsonFile(const nlohmann::json& j, const std::string& filename)
{
	std::ofstream out(filename);
	out << std::setw(4) << j << std::endl;
	out.close();
}

} // namespace skybolt