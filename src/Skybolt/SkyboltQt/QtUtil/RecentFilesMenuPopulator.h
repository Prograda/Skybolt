/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QMenu>
#include <QSettings>
#include <functional>

class RecentFilesMenuPopulator
{
public:
	typedef std::function<void(const QString&)> FileOpener;

	RecentFilesMenuPopulator(QMenu* menu, QSettings* settings, FileOpener fileOpener);

	void addFilename(const QString &fileName);

private:
	void updateRecentFileActions();

private:
	QMenu* menu;
	QSettings* settings;
	FileOpener fileOpener;
	std::vector<QAction*> recentFileActs;
	static const int maxRecentFiles = 10;
};
