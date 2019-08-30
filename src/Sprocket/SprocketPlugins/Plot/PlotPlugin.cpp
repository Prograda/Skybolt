/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlotCurve.h"
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/PlotColors.h>
#include <Sprocket/DataSeries/DataSeries.h>

#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_canvas.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_series_data.h>
#include <qwt/qwt_plot_zoomer.h>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>

class ReusableIdAssigner
{
public:
	ReusableIdAssigner() :
		maxUnusedId(0)
	{
	}

	size_t assign()
	{
		if (!usedIds.empty())
		{
			size_t id = *usedIds.begin();
			usedIds.erase(id);
			return id;
		}
		return maxUnusedId++;
	}

	void unassign(size_t id)
	{
		usedIds.insert(id);
	}

private:
	std::set<size_t> usedIds;
	size_t maxUnusedId;
};

class PlotColorAssigner
{
public:
	QColor assign(void* p)
	{
		const auto& plotColors = getPlotColors();

		size_t id = assigner.assign();
		ids[p] = id;
		return plotColors[id % plotColors.size()];
	}

	void unassign(void* p)
	{
		auto it = ids.find(p);
		assert(it != ids.end());
		assigner.unassign(it->second);
		ids.erase(it);
	}

private:
	ReusableIdAssigner assigner;
	std::map<void*, size_t> ids;
};

typedef std::shared_ptr<PlotCurve> PlotCurvePtr;

class DataSeriesCurveMaterializer
{
public:
	DataSeriesCurveMaterializer(const std::shared_ptr<DataSeries>& dataSeries, QwtPlot* plot, const std::string& keyX) :
		mDataSeries(dataSeries),
		mPlot(plot),
		mKeyX(keyX)
	{
		assert(mDataSeries);
		assert(mPlot);

		sync();

		mConnections.push_back(dataSeries->keyAdded.connect([this, keyX] (const std::string& keyY) {
			sync();
		}));

		mConnections.push_back(dataSeries->keyRemoved.connect([this, keyX, dataSeries] (const std::string& keyY) {
			sync();
		}));

	};

private:
	void sync()
	{
		std::set<std::string> keys = getDesiredKeys();
		// Remove old keys
		std::set<std::string> keysToRemove;
		for (const auto& item : mCurves)
		{
			if (keys.find(item.first) == keys.end())
			{
				keysToRemove.insert(item.first);
			}
		}
		for (const auto& key : keysToRemove)
		{
			removeCurve(key);
		}

		// Add new keys
		for (const auto& key : keys)
		{
			if (mCurves.find(key) == mCurves.end())
			{
				addCurve(key);
			}
		}
	}

	std::set<std::string> getDesiredKeys() const
	{
		std::set<std::string> result;
		if (mDataSeries->data.find(mKeyX) != mDataSeries->data.end())
		{
			for (const auto& item : mDataSeries->data)
			{
				std::string keyY = item.first;
				if (mKeyX != keyY)
				{
					result.insert(keyY);
				}
			}
		}
		return result;
	}

	void addCurve(const std::string& keyY)
	{
		QwtPlotCurve* curve = new QwtPlotCurve(QString::fromStdString(keyY));// Will be auto deleted by QwtPlot
		curve->setPen(mPlotColorAssigner.assign(curve));

		curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
		curve->attach(mPlot); 

		mCurves[keyY] = std::make_shared<PlotCurve>(curve, mDataSeries, mKeyX, keyY);
	}

	void removeCurve(const std::string& keyY)
	{
		auto it = mCurves.find(keyY);
		assert(it != mCurves.end());
		if (it != mCurves.end())
		{
			auto qwtCurve = it->second->getQwtCurve();
			qwtCurve->detach();
			mPlotColorAssigner.unassign(qwtCurve);
			mCurves.erase(it);
		}
	}
	
private:
	PlotColorAssigner mPlotColorAssigner;
	std::shared_ptr<DataSeries> mDataSeries;
	QwtPlot* mPlot;
	std::string mKeyX;
	std::map<std::string, PlotCurvePtr> mCurves;
	std::vector<boost::signals2::scoped_connection> mConnections;
};

class DataSeriesRegistryListener : public RegistryListener<NamedDataSeries>
{
public:
	DataSeriesRegistryListener(QwtPlot* plot, const std::string& keyX) :
		mPlot(plot),
		mKeyX(keyX)
	{
		assert(mPlot);
	}

	void itemAdded(const std::shared_ptr<NamedDataSeries>& item) override
	{
		auto data = item->data;
		mMaterializers[data] = std::make_shared<DataSeriesCurveMaterializer>(data, mPlot, mKeyX);
	}

	void itemAboutToBeRemoved(const std::shared_ptr<NamedDataSeries>& item) override
	{
		mMaterializers.erase(item->data);
	}

private:
	QwtPlot* mPlot;
	std::map<DataSeriesPtr, std::shared_ptr<DataSeriesCurveMaterializer>> mMaterializers;
	std::string mKeyX;
};

class PlotPlugin : public EditorPlugin
{
public:
	PlotPlugin(const EditorPluginConfig& config) :
		mDataSeriesRegistry(config.dataSeriesRegistry)
	{
		assert(mDataSeriesRegistry);

		auto plot = new QwtPlot;
		plot->setAutoReplot(true);
		plot->setAxisAutoScale(QwtPlot::yLeft);
		plot->setAxisAutoScale(QwtPlot::xBottom);
		plot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		
		{ // visual styling
			QPalette pal = plot->palette();
			plot->setPalette(pal);
			plot->setAutoFillBackground(true);
			static_cast<QwtPlotCanvas*>(plot->canvas())->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
		}

		QwtLegend* legend = new QwtLegend(plot);
		plot->insertLegend(legend);

		QwtPlotZoomer* zoomer = new QwtPlotZoomer(plot->canvas());
		QObject::connect(zoomer, &QwtPlotZoomer::zoomed, [=](const QRectF&)
		{
			if (zoomer->zoomRectIndex() == 0)
			{
				plot->setAxisAutoScale(zoomer->xAxis());
				plot->setAxisAutoScale(zoomer->yAxis());
				plot->replot();
			}
		});

		mDataRegistryListener = std::make_unique<DataSeriesRegistryListener>(plot, "t");
		mDataSeriesRegistry->addListener(mDataRegistryListener.get());

		zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton);

		mToolWindow.name = "Plot";
		mToolWindow.widget = plot;
	}

	~PlotPlugin() override
	{
		mDataSeriesRegistry->removeListener(mDataRegistryListener.get());
	}

	std::vector<ToolWindow> getToolWindows() override
	{
		return { mToolWindow };
	}

private:
	ToolWindow mToolWindow;
	std::shared_ptr<DataSeriesRegistry> mDataSeriesRegistry;
	std::unique_ptr<DataSeriesRegistryListener> mDataRegistryListener;
};

namespace plugins {

	std::shared_ptr<EditorPlugin> createEditorPlugin(const EditorPluginConfig& config)
	{
		return std::make_shared<PlotPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEditorPlugin,
		createEditorPlugin
	)
}
