/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

namespace skybolt::refl {

class Instance;
class Property;
class Type;
class TypeRegistry;

using TypePtr = std::shared_ptr<Type>;
using PropertyPtr = std::shared_ptr<Property>;

} // namespace skybolt::refl