/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Registry.h>

#include <memory>

namespace skybolt {
namespace vis {

class VisFactory
{
public:
	virtual ~VisFactory() {}
};

enum class VisFactoryType
{
	WaveHeightTextureGenerator
};

typedef std::shared_ptr<VisFactory> VisFactoryPtr;
typedef RegistryT<VisFactoryType, VisFactoryPtr> VisFactoryRegistry;
typedef std::shared_ptr<VisFactoryRegistry> VisFactoryRegistryPtr;

void addDefaultFactories(VisFactoryRegistry& registry);

} // namespace vis
} // namespace skybolt
