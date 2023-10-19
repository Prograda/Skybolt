/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MainWindowUtil.h"
#include "MainWindow.h"
#include "EditorPlugin.h"
#include "Scenario/ScenarioWorkspace.h"

void addToolWindows(MainWindow& mainWindow, const std::vector<EditorPluginPtr>& plugins)
{
	for (const EditorPluginPtr& plugin : plugins)
	{
		auto windows = plugin->getToolWindows();
		for (const auto& window : windows)
		{
			mainWindow.addToolWindow(window.name, window.widget);
		}
	}
}