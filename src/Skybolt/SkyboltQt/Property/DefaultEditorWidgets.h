/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "PropertyEditorWidgetFactory.h"
#include <SkyboltReflection/SkyboltReflectionFwd.h>

//! If typeRegistry is provided, editing of complex reflected types (e.g. structs) is enabled. Otherwise only primitive types are supported.
PropertyEditorWidgetFactoryMap getDefaultEditorWidgetFactoryMap(skybolt::refl::TypeRegistry* typeRegistry = nullptr);