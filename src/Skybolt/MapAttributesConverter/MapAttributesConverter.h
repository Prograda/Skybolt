/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Image>

namespace skybolt {

typedef std::pair<int, osg::Vec4f> AttributeColor;
typedef std::vector<AttributeColor> AttributeColors;

osg::ref_ptr<osg::Image> convertAttributeMap(const osg::Image& image, const AttributeColors& srcAttributeColors);

} // namespace skybolt