/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RecentFilesMenuPopulator.h"
#include <QFileInfo>
#include <QDir>

static const QString recentFileListName = "recentFileList";

RecentFilesMenuPopulator::RecentFilesMenuPopulator(QMenu* menu, QSettings* settings, FileOpener fileOpener) :
	settings(settings),
	fileOpener(fileOpener)
{
	for (int i = 0; i < maxRecentFiles; ++i)
	{
		QAction* action = new QAction;
		action->setVisible(false);
		recentFileActs.push_back(action);
		menu->addAction(action);

		QObject::connect(action, &QAction::triggered, [fileOpener, action] {fileOpener(action->data().toString()); });
	}
	updateRecentFileActions();
}

void RecentFilesMenuPopulator::addFilename(const QString &filename)
{
	QString fixedFilename = QDir::fromNativeSeparators(filename);

	QStringList files = settings->value(recentFileListName).toStringList();
	files.removeAll(fixedFilename);
	files.prepend(fixedFilename);
	while (files.size() > maxRecentFiles)
	{
		files.removeLast();
	}
	settings->setValue(recentFileListName, files);

	updateRecentFileActions();
}

void RecentFilesMenuPopulator::updateRecentFileActions()
{
	QStringList files = settings->value(recentFileListName).toStringList();

	int numRecentFiles = qMin(files.size(), (int)maxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		QString text = QObject::tr("&%1 %2").arg(i + 1).arg(files[i]);
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < maxRecentFiles; ++j)
	{
		recentFileActs[j]->setVisible(false);
	}

	if (numRecentFiles == 0)
	{
		recentFileActs[0]->setVisible(true);
		recentFileActs[0]->setText("No recent files");
		recentFileActs[0]->setEnabled(false);
	}
	else
	{
		recentFileActs[0]->setEnabled(true);
	}
}
