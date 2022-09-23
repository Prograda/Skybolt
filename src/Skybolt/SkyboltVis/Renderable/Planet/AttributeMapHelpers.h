/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Image>
#include <utility>
#include <vector>

namespace skybolt {
namespace vis {

using AttributeColor = std::pair<int, osg::Vec4f>;
using AttributeColors = std::vector<AttributeColor>;

const AttributeColors& getNlcdAttributeColors();

//! Analyses each pixel of an albedo source image and returns an image
//! giving pixel's corresponding material ID. E.g a green pixel might be
//! mapped to an ID representing grass, a brown pixel mapped to an ID representing
//@ dirt etc.
osg::ref_ptr<osg::Image> convertToAttributeMap(const osg::Image& albedo);

//! Convert an image of different colored attribute areas to an image of attribute interger IDs
osg::ref_ptr<osg::Image> convertAttributeMap(const osg::Image& image, const AttributeColors& srcAttributeColors);

} // namespace vis
} // namespace skybolt
