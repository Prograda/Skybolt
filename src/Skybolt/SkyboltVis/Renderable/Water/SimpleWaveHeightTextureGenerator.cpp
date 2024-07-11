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

#include "SimpleWaveHeightTextureGenerator.h"

#include <osg/Image>
#include <osg/Texture2D>

namespace skybolt {
namespace vis {

SimpleWaveHeightTextureGenerator::SimpleWaveHeightTextureGenerator()
{
	int width = 512;
	int height = width;

	osg::Image* image = new osg::Image;
	image->allocateImage(width, height, 1, GL_RGB, GL_FLOAT);
	image->setInternalTextureFormat(GL_RGB32F_ARB);

	float* p = reinterpret_cast<float*>(image->data());
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
		}
	}

	mTexture = (new osg::Texture2D(image));
}

} // namespace vis
} // namespace skybolt
