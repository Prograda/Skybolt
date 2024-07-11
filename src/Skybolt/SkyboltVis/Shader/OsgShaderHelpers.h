/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Shader>
#include <optional>
#include <string>

namespace skybolt {
namespace vis {

std::string loadFileToString(const std::string& filepath);

osg::Shader* readShaderFile(osg::Shader::Type type, const std::string& filename);
osg::Shader* readShaderFromString(osg::Shader::Type type, const std::string& source, const std::optional<std::string>& includeDirPath = {});

} // namespace vis
} // namespace skybolt
