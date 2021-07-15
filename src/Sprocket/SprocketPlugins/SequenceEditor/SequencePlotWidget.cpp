/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SequencePlotWidget.h"
#include <Sprocket/PlotColors.h>
#include <SkyboltEngine/TimeSource.h>
#include <SkyboltEngine/Sequence/EntityStateSequenceController.h>
#include <QBoxLayout>
#include <QTimer>

#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_canvas.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_marker.h>
#include <qwt/qwt_plot_zoomer.h>
#include <qwt/qwt_series_data.h>
#include <qwt/qwt_symbol.h>

using namespace skybolt;

SequencePlotWidget::SequencePlotWidget(TimeSource* timeSource, QWidget* parent) :
	QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);

	mPlot = new QwtPlot;
	layout->addWidget(mPlot);

	mPlot->setAutoReplot(true);
	mPlot->setAxisAutoScale(QwtPlot::yLeft);
	mPlot->setAxisAutoScale(QwtPlot::xBottom);
	mPlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	{ // visual styling
		QPalette pal = mPlot->palette();
		mPlot->setPalette(pal);
		mPlot->setAutoFillBackground(true);
		static_cast<QwtPlotCanvas*>(mPlot->canvas())->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
	}

	QwtLegend* legend = new QwtLegend(mPlot);
	mPlot->insertLegend(legend);

	QwtPlotZoomer* zoomer = new QwtPlotZoomer(mPlot->canvas());
	QObject::connect(zoomer, &QwtPlotZoomer::zoomed, [=](const QRectF&)
	{
		if (zoomer->zoomRectIndex() == 0)
		{
			mPlot->setAxisAutoScale(zoomer->xAxis());
			mPlot->setAxisAutoScale(zoomer->yAxis());
			mPlot->replot();
		}
	});

	zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton);

	mTimeMarker = new QwtPlotMarker();
	mTimeMarker->setValue(0.0, 0.0);
	mTimeMarker->setLineStyle(QwtPlotMarker::VLine);
	mTimeMarker->setLinePen(Qt::darkGray, 0, Qt::SolidLine);
	mTimeMarker->attach(mPlot);

	mConnections.push_back(timeSource->timeChanged.connect([this](double time) {
		mTimeMarker->setValue(QPointF(time, 0.0));
	}));

	// Create timer to refresh the plot if data changed
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [this] {
		for (const auto& [curve, replotter] : mDirtyCurveReplotters)
		{
			replotter();
		}
		mDirtyCurveReplotters.clear();
	});
	timer->start(0);
}

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

	inline void insert(int i, const QPointF &point)
	{
		d_samples.insert(i, point);
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

class Curve
{
public:
	using ValueGetter = std::function<double(const SequenceState& state)>;

	Curve(const std::string& name, const ValueGetter& valueGetter) :
		name(name), valueGetter(valueGetter) {}

	std::string name;
	ValueGetter valueGetter;
};

std::vector<Curve> getCurves(const std::shared_ptr<EntityStateSequence>& sequence)
{
	return {
		Curve("x", [=](const SequenceState& state) { return static_cast<const EntitySequenceState&>(state).position.x; }),
		Curve("y", [=](const SequenceState& state) { return static_cast<const EntitySequenceState&>(state).position.y; }),
		Curve("z", [=](const SequenceState& state) { return static_cast<const EntitySequenceState&>(state).position.z; })
	};
}

static void replotCurve(CurveData& data, const StateSequenceControllerPtr& controller, const Curve::ValueGetter& getter)
{
	data.clear();
	const auto& sequence = controller->getSequence();
	if (sequence->times.size() >= 2)
	{
		double firstTime = sequence->times.front();
		double lastTime = sequence->times.back();
		double range = lastTime - firstTime;

		static const size_t pointCount = 100;
		for (size_t i = 0; i < pointCount; ++i)
		{
			double time = glm::mix(firstTime, lastTime, double(i) / double(pointCount - 1));
			auto state = controller->getStateAtTime(time);

			data.append(QPointF(time, getter(*state)));
		}
	}
}

using PointMarkers = std::vector<QwtPlotMarker*>;

static void replotPoints(QwtPlot* plot, const std::shared_ptr<PointMarkers>& markers, const SequencePtr& sequence, const Curve::ValueGetter& getter)
{
	for (QwtPlotMarker* marker : *markers)
	{
		marker->detach();
	}
	markers->clear();

	for (size_t i = 0; i < sequence->times.size(); ++i)
	{
		double time = sequence->times[i];
		double value = getter(sequence->getItemAtIndex(i));

		QwtPlotMarker* marker = new QwtPlotMarker();
		marker->setValue(QPointF(time, value));
		marker->setLineStyle(QwtPlotMarker::NoLine);
		marker->setSymbol(new QwtSymbol(QwtSymbol::Diamond,
			QColor(Qt::white), QColor(Qt::white), QSize(4, 4)));
		marker->attach(plot);
		
		markers->push_back(marker);
	}
}

void SequencePlotWidget::setSequenceController(const skybolt::StateSequenceControllerPtr& controller)
{
	// Remove previous plot items
	mConnections.clear();

	for (QwtPlotCurve* curve : mCurves)
	{
		curve->detach();
	}

	for (auto& markers : mCurvePointMakers)
	{
		for (auto& marker : *markers)
		{
			marker->detach();
		}
	}

	// Add new plot items
	const auto& sequence = controller->getSequence();

	std::vector<Curve> curves;
	if (auto entityStateSequence = std::dynamic_pointer_cast<EntityStateSequence>(sequence))
	{
		curves = getCurves(entityStateSequence);
	}
	else
	{
		curves = {};
	}

	const auto& plotColors = getPlotColors();

	int curveIndex = 0;
	for (const auto& c : curves)
	{
		CurveData* data = new CurveData();

		QwtPlotCurve* curve = new QwtPlotCurve(QString::fromStdString(c.name)); // Will be auto deleted by QwtPlot
		curve->setPen(plotColors[curveIndex % plotColors.size()]);
		curve->setData(data);
		curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
		curve->attach(mPlot);

		mCurves.push_back(curve);

		auto pointMarkers = std::make_shared<PointMarkers>();
		mCurvePointMakers.push_back(pointMarkers);

		auto replotter = [=] {
			replotCurve(*data, controller, c.valueGetter);
			replotPoints(mPlot, pointMarkers, sequence, c.valueGetter);
		};

		// Add existing items
		mDirtyCurveReplotters[curve] = replotter;

		// Listen to change events
		mConnections.push_back(sequence->itemAdded.connect([=](size_t index) {
			mDirtyCurveReplotters[curve] = replotter;
		}));

		mConnections.push_back(sequence->valueChanged.connect([=](size_t index) {
			mDirtyCurveReplotters[curve] = replotter;
		}));

		mConnections.push_back(sequence->itemRemoved.connect([=](size_t index) {
			mDirtyCurveReplotters[curve] = replotter;
		}));

		++curveIndex;
	}
}