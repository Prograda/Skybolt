/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include <glm/glm.hpp>
#include <utility>
#include <vector>

namespace skybolt {
namespace vis {

using AttributeColor = std::pair<int, glm::vec4>;
using AttributeColors = std::vector<AttributeColor>;

const AttributeColors& getNlcdAttributeColors();

//! Convert an image of different colored attribute areas to an image of attribute interger IDs
ImagePtr convertAttributeMap(const ImageFactory& imageFactory, const Image& image, const AttributeColors& srcAttributeColors);

} // namespace vis
} // namespace skybolt
