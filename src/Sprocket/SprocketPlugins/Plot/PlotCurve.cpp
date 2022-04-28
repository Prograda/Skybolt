/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlotCurve.h"
#include <Sprocket/DataSeries/DataSeries.h>

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

#include <assert.h>
#include <set>

using namespace skybolt;

class CurveData : public QwtArraySeriesData<QPointF>
{
public:
	CurveData() :
		dirtyBounds(true)
	{
	}

	virtual QRectF boundingRect() const
	{
		if (dirtyBounds)
		{
			dirtyBounds = false;
			d_boundingRect = qwtBoundingRect(*this);
		}

		return d_boundingRect;
	}

	inline void append(const QPointF &point)
	{
		d_samples += point;
		dirtyBounds = true;

	}

	inline void remove(int i)
	{
		d_samples.erase(d_samples.begin() + i);
		dirtyBounds = true;
	}

	inline void replace(int i, const QPointF &point)
	{
		d_samples[i] = point;
		dirtyBounds = true;
	}

	void clear()
	{
		d_samples.clear();
		d_samples.squeeze();
		dirtyBounds = true;
	}
private:
	mutable bool dirtyBounds;
};

PlotCurve::PlotCurve(QwtPlotCurve* curve, const DataSeriesPtr& dataSeries, const std::string& keyX, const std::string& keyY) :
	mQwtCurve(curve)
{
	assert(mQwtCurve);
	CurveData* data = new CurveData(); // Will be auto deleted by QwtPlotCurve

	auto evalIfKeysPresent = [=](const std::string& keyX, const std::string& keyY, const std::function<void(const DataSeries::DoubleVector& dataX, const DataSeries::DoubleVector& dataY)>& fn) {
		auto dataX = dataSeries->data.find(keyX);
		auto dataY = dataSeries->data.find(keyY);
		if (dataX != dataSeries->data.end() && dataY != dataSeries->data.end())
		{
			fn(dataX->second, dataY->second);
		}
	};

	evalIfKeysPresent(keyX, keyY, [data] (const auto& dataX, const auto& dataY) {
		size_t count = std::min(dataX.size(), dataY.size());
		for (size_t i = 0; i < count; ++i)
		{
			data->append(QPointF(dataX[i], dataY[i]));
		}
	});

	curve->setData(data);

	mConnections.push_back(dataSeries->valuesAdded.connect([=](const IntRangeInclusive& range) {
		evalIfKeysPresent(keyX, keyY, [range, curve](const auto& dataX, const auto& dataY) {
			for (int i = range.first; i <= range.last; ++i)
			{
				static_cast<CurveData*>(curve->data())->append(QPointF(dataX[i], dataY[i]));
			}
		});
	}));

	mConnections.push_back(dataSeries->valuesChanged.connect([=](const IntRangeInclusive& range) {
		evalIfKeysPresent(keyX, keyY, [range, curve](const auto& dataX, const auto& dataY) {
			for (int i = range.first; i <= range.last; ++i)
			{
				static_cast<CurveData*>(curve->data())->replace(i, QPointF(dataX[i], dataY[i]));
			}
		});
	}));

	mConnections.push_back(dataSeries->valuesRemoved.connect([=](const IntRangeInclusive& range) {
		for (int i = range.first; i <= range.last; ++i)
		{
			static_cast<CurveData*>(curve->data())->remove(i);
		}
	}));
}
