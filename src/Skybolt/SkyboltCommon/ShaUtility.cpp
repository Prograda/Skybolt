/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ShaUtility.h"

#include <boost/uuid/detail/sha1.hpp>

namespace skybolt {

// From https://stackoverflow.com/questions/28489153/how-to-portably-compute-a-sha1-hash-in-c
std::string calcSha1(const std::string& p_arg)
{
	boost::uuids::detail::sha1 sha1;
	sha1.process_bytes(p_arg.data(), p_arg.size());
	unsigned hash[5] = { 0 };
	sha1.get_digest(hash);

	// Back to string
	char buf[41] = { 0 };

	for (int i = 0; i < 5; i++)
	{
		snprintf(buf + (i << 3), 41, "%08x", hash[i]);
	}

	return std::string(buf);
}

} // namespace skybolt