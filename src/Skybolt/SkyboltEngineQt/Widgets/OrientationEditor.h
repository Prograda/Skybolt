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
