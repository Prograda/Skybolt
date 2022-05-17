/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ShadowHelpers.h"
#include "OsgStateSetHelpers.h"

namespace skybolt {
namespace vis {

void addShadowMapsToStateSet(const ShadowMaps& shadowMaps, osg::StateSet& stateSet, int firstTextureUnitIndex)
{
	stateSet.setDefine("SHADOW_CASCADE_COUNT", std::to_string(shadowMaps.size()));

	osg::Uniform* uniform = new osg::Uniform(osg::Uniform::SAMPLER_2D_SHADOW, "shadowSampler", (int)shadowMaps.size());
	for (int i = 0; i < (int)shadowMaps.size(); ++i)
	{
		stateSet.setTextureAttributeAndModes(firstTextureUnitIndex, shadowMaps[i]);
		uniform->setElement(i, firstTextureUnitIndex++);
	}
	stateSet.addUniform(uniform);
}

} // namespace vis
} // namespace skybolt
