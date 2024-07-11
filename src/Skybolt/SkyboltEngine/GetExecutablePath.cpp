/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GetExecutablePath.h"
#include "ThirdParty/whereami.h"

namespace skybolt {

file::Path getExecutablePath()
{
	file::Path result;

	char* path = NULL;
	int length, dirname_length;

	length = wai_getExecutablePath(NULL, 0, &dirname_length);
	if (length > 0)
	{
		path = (char*)malloc(length + 1);
		if (path)
		{
			wai_getExecutablePath(path, length, &dirname_length);
			path[length] = '\0';
			result = path;
		
			free(path);
		}
	}
	return result.parent_path();
}

} // namespace skybolt