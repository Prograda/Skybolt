/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltWidgets/Property/DefaultEditorWidgets.h>
#include <SkyboltWidgets/Property/PropertyEditorWidgetFactory.h>

std::unique_ptr<skybolt::PropertyEditorWidgetFactoryMap> createSkyboltEditorWidgetFactoryMap(const skybolt::DefaultEditorWidgetFactoryMapConfig& config);