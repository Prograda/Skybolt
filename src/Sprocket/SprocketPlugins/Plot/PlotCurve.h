/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <Sprocket/SprocketFwd.h>
#include <boost/signals2.hpp>

class QwtPlotCurve;

class PlotCurve
{
public:
	PlotCurve(QwtPlotCurve* curve, const DataSeriesPtr& dataSeries, const std::string& keyX, const std::string& keyY);

	QwtPlotCurve* getQwtCurve() const { return mQwtCurve; }

private:
	std::vector<boost::signals2::scoped_connection> mConnections;
	QwtPlotCurve* mQwtCurve;
};
