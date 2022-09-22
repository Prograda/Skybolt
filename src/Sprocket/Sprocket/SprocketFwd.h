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
struct DataSeries;
class PropertiesModel;
class PythonScript;
class Script;

typedef std::shared_ptr<DataSeries> DataSeriesPtr;
typedef std::shared_ptr<EditorPlugin> EditorPluginPtr;
typedef std::shared_ptr<EntityChooserDialogFactory> EntityChooserDialogFactoryPtr;
typedef std::shared_ptr<PropertiesModel> PropertiesModelPtr;
typedef std::shared_ptr<Script> ScriptPtr;

typedef std::function<EditorPluginPtr(const EditorPluginConfig&)> EditorPluginFactory;
