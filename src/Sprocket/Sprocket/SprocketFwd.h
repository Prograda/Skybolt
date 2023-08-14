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
class JsonProjectSerializable;
class OsgWidget;
class PropertyEditor;
class PropertiesModel;
class ScenarioObject;
class SceneSelectionModel;
struct ScenarioObjectType;
class ScenarioTreeWidget;
class TreeItem;
class ViewportInput;
class ViewportMouseEventHandler;
class ViewportWidget;

using EditorPluginPtr = std::shared_ptr<EditorPlugin>;
using EntityChooserDialogFactoryPtr = std::shared_ptr<EntityChooserDialogFactory>;
using PropertiesModelPtr = std::shared_ptr<PropertiesModel>;
using ScenarioObjectPtr = std::shared_ptr<ScenarioObject>;
using ScenarioObjectTypePtr = std::shared_ptr<ScenarioObjectType>;
using TreeItemPtr = std::shared_ptr<TreeItem>;
using ViewportMouseEventHandlerPtr = std::shared_ptr<ViewportMouseEventHandler>;

using EditorPluginFactory = std::function<EditorPluginPtr(const EditorPluginConfig&)>;
