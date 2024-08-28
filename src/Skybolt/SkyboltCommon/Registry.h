/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <map>

namespace skybolt {

class Registry
{
public:
	virtual ~Registry() = default;
};

template <typename KeyT, typename FactoryT>
class RegistryT : public std::map<KeyT, FactoryT>, public Registry
{
public:
	~RegistryT() override = default;
};

} // namespace skybolt