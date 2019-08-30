/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "UiApplication.h"

#include <QMessageBox>
#include <QtOpenGL/QGLFormat>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>

#include <iostream>


UiApplication::UiApplication(int argc, char* argv[]) :
	QApplication(argc, argv)
{
}

// Override QApplication::notify() so we can catch exceptions thrown from the Qt event loop
bool UiApplication::notify(QObject *rec, QEvent *ev)
{
	try
	{
		return QApplication::notify(rec, ev);
	}
	catch (std::exception &e)
	{
		printError(e.what());
	}
	return false;
}

void UiApplication::update()
{
}

void UiApplication::printError(const std::string &error)
{
	std::cout << "Error: " << error << std::endl;
	// Linux TODO
	#ifdef WIN32
	MessageBox(NULL, error.c_str(), "Exception", 0);
	#endif
}
bool UiApplication::supportsOpenGl()
{
	return QGLFormat::hasOpenGL();
}
