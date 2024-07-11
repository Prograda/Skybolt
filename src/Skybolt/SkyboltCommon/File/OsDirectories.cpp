/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsDirectories.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace skybolt {
namespace file {

Path getHomeDirectory()
{
#ifdef unix
	return getenv("HOME");
#elif defined(WIN32)
	const char* homeDrive = std::getenv("HOMEDRIVE");
	const char* homePath = std::getenv("HOMEPATH");
	return std::string(homeDrive) + std::string(homePath);
#else
#error "Unsupported operating system"
#endif
}

Path getAppUserDataDirectory(const std::string& applicationName)
{
#ifdef unix
	return getHomeDirectory()
		.append(applicationName); // TODO: where should this go on Unix?
#elif defined(WIN32)
	return getHomeDirectory()
		.append("AppData")
		.append("Local")
		.append(applicationName);
#else
#error "Unsupported operating system"
#endif
	
}

} // namespace file
} // namespace skybolt
