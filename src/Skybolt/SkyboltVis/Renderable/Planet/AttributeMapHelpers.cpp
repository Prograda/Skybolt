/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AttributeMapHelpers.h"
#include <MapAttributesConverter/MapAttributesConverter.h>

namespace skybolt {
namespace vis {

const AttributeColors& getNlcdAttributeColors()
{
	static AttributeColors c = {
		AttributeColor(0, osg::Vec4f(0.27843137255f, 0.41960784314f, 0.62745098039f, 1)), // water
		AttributeColor(0, osg::Vec4f(0.69803921569f, 0.67843137255f, 0.63921568628f, 1)), // barren
		// developed
		AttributeColor(0, osg::Vec4f(0.86666666667f, 0.78823529412f, 0.78823529412f, 1)), // developed, open space
		AttributeColor(0, osg::Vec4f(0.84705882353f, 0.57647058824f, 0.50980392157f, 1)), // developed, low
		AttributeColor(3, osg::Vec4f(0.92941176471f, 0.00000000000f, 0.00000000000f, 1)), // developed, med
		AttributeColor(4, osg::Vec4f(0.66666666667f, 0.00000000000f, 0.00000000000f, 1)), // developed, high
		// scrub
		AttributeColor(5, osg::Vec4f(0.64705882353f, 0.54901960784f, 0.18823529412f, 1)), // dwarf scrub
		AttributeColor(5, osg::Vec4f(0.80000000000f, 0.72941176471f, 0.48627450980f, 1)), // shrub scrub
		// grass
		AttributeColor(6, osg::Vec4f(0.88627450980f, 0.88627450980f, 0.75686274510f, 1)), // grassland
		AttributeColor(6, osg::Vec4f(0.78823529412f, 0.78823529412f, 0.46666666667f, 1)), // sedge
		AttributeColor(6, osg::Vec4f(0.60000000000f, 0.75686274510f, 0.27843137255f, 1)), // lichens
		AttributeColor(6, osg::Vec4f(0.46666666667f, 0.67843137255f, 0.57647058824f, 1)), // moss
		AttributeColor(6, osg::Vec4f(0.72941176471f, 0.84705882353f, 0.91764705882f, 1)), // woody wetlands
		AttributeColor(6, osg::Vec4f(0.43921568628f, 0.63921568628f, 0.72941176471f, 1)), // herbaceous wetlands
		// pasture
		AttributeColor(7, osg::Vec4f(0.85882352941f, 0.84705882353f, 0.23921568628f, 1)), // pasture
		// crops
		AttributeColor(8, osg::Vec4f(0.66666666667f, 0.43921568628f, 0.15686274510f, 1)), // cultivated crops
		// forest
		AttributeColor(9, osg::Vec4f(0.40784313726f, 0.66666666667f, 0.38823529412f, 1)), // deciduous forest
		AttributeColor(9, osg::Vec4f(0.10980392157f, 0.38823529412f, 0.18823529412f, 1)), // evergreen forest
		AttributeColor(9, osg::Vec4f(0.70980392157f, 0.78823529412f, 0.55686274510f, 1)), // mixed forest
		// snow
		AttributeColor(10, osg::Vec4f(0.81960784314f, 0.86666666667f, 0.97647058824f, 1)) // ice and snow
	};

	return c;
}

} // namespace vis
} // namespace skybolt
