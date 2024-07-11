/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <QWidget>

class QStackedWidget;

class OrientationEditor : public QWidget
{
public:
	OrientationEditor(QWidget* parent = nullptr);

	void setOrientation(const skybolt::sim::OrientationPtr& orientation, const skybolt::sim::LatLon& latLon);

	skybolt::sim::OrientationPtr getOrientation();

private:
	class Orientable* getCurrentEditor() const;

private:
	QStackedWidget* mStackedWidget;
	skybolt::sim::LatLon mLatLon = skybolt::sim::LatLon(0,0);
};
