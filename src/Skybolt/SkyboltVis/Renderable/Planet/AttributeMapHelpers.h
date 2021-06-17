/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <MapAttributesConverter/MapAttributesConverter.h>

namespace skybolt {
namespace vis {

const AttributeColors& getNlcdAttributeColors();

//! Analyses each pixel of an albedo source image and returns an image
//! giving pixel's corresponding material ID. E.g a green pixel might be
//! mapped to an ID representing grass, a brown pixel mapped to an ID representing
//@ dirt etc.
osg::ref_ptr<osg::Image> convertToAttributeMap(const osg::Image& albedo);

} // namespace vis
} // namespace skybolt
