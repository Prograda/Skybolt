/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FindPython.h"

#include <string>
#include <vector>

// FIXME: Required to fix boost process compiler warning
#ifdef WIN32
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0601 // windows 7 - minimum supported operating system version
	#endif
#endif

#include <boost/process.hpp>


bool isPythonOnPath(unsigned int majorVersion, unsigned int minorVersion)
{
	namespace bp = boost::process;
	// Python executable names on windows and linux.
	// Note that we don't include "py" as it only redirects to python - finding py does not mean Python is on the PATH.
	std::string majorVersionString = std::to_string(majorVersion);
	std::string majorMinorVersionString =  majorVersionString + "." + std::to_string(minorVersion);
	std::vector<std::string> commands = {"python", "python" + majorVersionString, "python" + majorMinorVersionString};

	for (const auto& command : commands)
	{
		std::string result;
		bp::ipstream output;
		try
		{
			// Execute command and capture standard output
			bp::child c(command + " --version", bp::std_out > output, bp::std_err > bp::null);
			std::getline(output, result);
			c.wait(); // wait for process to exit

			if (result.find("Python " + majorMinorVersionString) != std::string::npos)
			{
				return true;  // Found Python
			}
		} catch (const std::exception& e)
		{
		}
	}

	return false;
}