/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <boost/signals2.hpp>
#include <QWidget>

class SequencePlotWidget : public QWidget
{
public:
	SequencePlotWidget(skybolt::TimeSource* timeSource, QWidget* parent = nullptr);

	void setSequenceController(const skybolt::StateSequenceControllerPtr& controller);

private:
	class QwtPlot* mPlot;
	class QwtPlotMarker* mTimeMarker;
	std::vector<class QwtPlotCurve*> mCurves;

	using PointMarkers = std::vector<QwtPlotMarker*>;
	std::vector<std::shared_ptr<PointMarkers>> mCurvePointMakers;

	std::vector<boost::signals2::connection> mConnections;
	std::map<class QwtPlotCurve*, std::function<void()>> mDirtyCurveReplotters;
};