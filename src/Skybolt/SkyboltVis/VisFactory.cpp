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

#include "VisFactory.h"
#include "Renderable/Water/SimpleWaveHeightTextureGenerator.h"

namespace skybolt {
namespace vis {

void addDefaultFactories(VisFactoryRegistry& registry)
{
	registry[VisFactoryType::WaveHeightTextureGenerator] = std::make_shared<SimpleWaveHeightTextureGeneratorFactory>();
}

} // namespace vis
} // namespace skybolt
