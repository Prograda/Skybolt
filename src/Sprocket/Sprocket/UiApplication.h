/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include <QApplication>

//! Main application class
class UiApplication : public QApplication
{
	Q_OBJECT

public:
	UiApplication(int argc, char* argv[]);

	//! Process Qt events
	bool notify(QObject *rec, QEvent *ev);

	static void printError(const std::string &error);

	static bool supportsOpenGl();

private slots:
	//! GUI per-frame update event
	void update();
};
