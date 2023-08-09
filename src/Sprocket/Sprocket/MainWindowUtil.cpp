/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MainWindowUtil.h"
#include "MainWindow.h"
#include "JsonProjectSerializable.h"
#include "EditorPlugin.h"

void connectJsonProjectSerializable(MainWindow& mainWindow, JsonProjectSerializable& serializable)
{
	QObject::connect(&mainWindow, &MainWindow::projectCleared, [serializable = &serializable] { serializable->resetProject(); });
	QObject::connect(&mainWindow, &MainWindow::projectLoaded, [serializable = &serializable] (const nlohmann::json& json) { serializable->readProject(json); });
	QObject::connect(&mainWindow, &MainWindow::projectSaved, [serializable = &serializable] (nlohmann::json& json) { serializable->writeProject(json); });
}

void addToolWindows(MainWindow& mainWindow, const std::vector<EditorPluginPtr>& plugins)
{
	for (const EditorPluginPtr& plugin : plugins)
	{
		auto windows = plugin->getToolWindows();
		for (const auto& window : windows)
		{
			mainWindow.addToolWindow(window.name, window.widget);
			connectJsonProjectSerializable(mainWindow, *plugin);
		}
	}
}