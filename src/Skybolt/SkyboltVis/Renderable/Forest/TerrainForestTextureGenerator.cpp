/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TerrainForestTextureGenerator.h"
#include "SkyboltVis/OsgImageHelpers.h"

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::Image> TerrainForestTextureGenerator::generate(const osg::Image& billboardTexture)
{
	osg::Vec4f color = averageSrgbColor(billboardTexture);

	osg::ref_ptr<osg::Image> image(new osg::Image);
	const int dim = 1;
	image->allocateImage(dim, dim, 1, GL_RGB, GL_UNSIGNED_BYTE);
	setPixelColor(*image, 0, 0, color);
	return image;
}

} // namespace vis
} // namespace skybolt
