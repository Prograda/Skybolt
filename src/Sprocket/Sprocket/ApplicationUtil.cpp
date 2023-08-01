/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ApplicationUtil.h"

#include <QMessageBox>
#include <QtOpenGL/QGLFormat>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>

#include <iostream>

void displayApplicationError(const std::string &error)
{
	std::cout << "Error: " << error << std::endl;
	// Linux TODO
	#ifdef WIN32
	MessageBox(NULL, error.c_str(), "Exception", 0);
	#endif
}


bool applicationSupportsOpenGl()
{
	return QGLFormat::hasOpenGL();
};
