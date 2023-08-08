/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>
#include <memory>

class EditorPlugin;
class EntityChooserDialogFactory;
struct EditorPluginConfig;
class PropertiesModel;

typedef std::shared_ptr<EditorPlugin> EditorPluginPtr;
typedef std::shared_ptr<EntityChooserDialogFactory> EntityChooserDialogFactoryPtr;
typedef std::shared_ptr<PropertiesModel> PropertiesModelPtr;

typedef std::function<EditorPluginPtr(const EditorPluginConfig&)> EditorPluginFactory;
