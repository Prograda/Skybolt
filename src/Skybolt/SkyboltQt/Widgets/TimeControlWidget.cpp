/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TimeControlWidget.h"
#include "TimeRateDialog.h"
#include "Icon/SkyboltIcons.h"
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QToolBar>

using namespace skybolt;

TimeControlWidget::TimeControlWidget(QWidget* parent) :
	QWidget(parent)
{
	mPlayAction = new QAction(getSkyboltIcon(SkyboltIcon::Play), tr("Play"), this);
	mPlayAction->setShortcut(tr("Ctrl+P"));

	mForwardAction = new QAction(getSkyboltIcon(SkyboltIcon::FastForward), tr("Forward"), this);
	mForwardAction->setShortcut(tr("Ctrl+F"));
	
	mBackwardAction = new QAction(getSkyboltIcon(SkyboltIcon::FastRewind), tr("Backward"), this);
	mBackwardAction->setShortcut(tr("Ctrl+B"));

	auto rateAction = new QAction(getSkyboltIcon(SkyboltIcon::Speed), tr("Rate"), this);

	mTimeRateLabel = new QLabel("", this);
	mTimeRateLabel->setVisible(false);

	QToolBar* bar = new QToolBar(this);
	bar->addAction(mBackwardAction);
	bar->addAction(mPlayAction);
	bar->addAction(mForwardAction);
	bar->addAction(rateAction);

	QWidget* rateActionWidget = bar->widgetForAction(rateAction);
	connect(rateAction, &QAction::triggered, this, [this, rateActionWidget] {
		if (!mTimeRateDialog && mRequestedTimeRateSource)
		{
			mTimeRateDialog = std::make_unique<TimeRateDialog>(mRequestedTimeRateSource->get(), this);
			mTimeRateDialog->move(rateActionWidget->mapToGlobal(QPoint(20, -50)));

			connect(mTimeRateDialog.get(), &TimeRateDialog::rateChanged, this, [this] (double newRate) {
				if (mRequestedTimeRateSource)
				{
					mRequestedTimeRateSource->set(newRate);
				}
			});

			connect(mTimeRateDialog.get(), &TimeRateDialog::closed, this, [this] {
				mTimeRateDialog.reset();
			});
			mTimeRateDialog->show();
		}		
	});

	auto layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(bar);
	layout->addWidget(mTimeRateLabel, 0);
	layout->addStretch();

	setEnabled(false);
}

TimeControlWidget::~TimeControlWidget() = default;

void TimeControlWidget::setTimeSource(TimeSource* source)
{
	mSource = source;
	setEnabled(source != nullptr);

	// Disconnect from previous source
	for (const QMetaObject::Connection& connection : mQtTimeSourceConnections)
	{
		QObject::disconnect(connection);
	}
	mQtTimeSourceConnections.clear();

	if (mSource)
	{
		// Connect to new source
		mQtTimeSourceConnections.push_back(connect(mPlayAction, &QAction::triggered, [&]()
		{
			if (mSource->getState() == TimeSource::StatePlaying)
			{
				mSource->setState(TimeSource::StateStopped);
			}
			else
			{
				mSource->setState(TimeSource::StatePlaying);
			}
		}));

		mQtTimeSourceConnections.push_back(connect(mForwardAction, &QAction::triggered, [&]() { mSource->setTime(mSource->getRange().end); }));
		mQtTimeSourceConnections.push_back(connect(mBackwardAction, &QAction::triggered, [&]() { mSource->setTime(mSource->getRange().start); }));

		mBoostTimeSourceConnection = mSource->stateChanged.connect([&](const TimeSource::State& state) {
			switch (state)
			{
			case TimeSource::StatePlaying:
				mPlayAction->setIcon(getSkyboltIcon(SkyboltIcon::Pause));
				mTimeRateLabel->setVisible(true);
				break;
			case TimeSource::StateStopped:
				mPlayAction->setIcon(getSkyboltIcon(SkyboltIcon::Play));
				mTimeRateLabel->setVisible(false);
				break;
			default:
				break;
			}
		});
	}
}

void TimeControlWidget::setTimelineMode(TimelineMode timelineMode)
{
	if (mTimelineMode != timelineMode)
	{
		mTimelineMode = timelineMode;
		mForwardAction->setVisible(timelineMode == TimelineMode::Free);
		mBackwardAction->setVisible(timelineMode == TimelineMode::Free);
	}
}

void TimeControlWidget::setRequestedTimeRateSource(ObservableValueD* source)
{
	mRequestedTimeRateSource = source;
	mBoostRequestedTimeRateSourceConnection = connectObservableValue(source, [&](const double& oldValue, const double& newValue) {
		updateTimeRateLabel();
		});
}

void TimeControlWidget::setActualTimeRateSource(skybolt::ObservableValueD* source)
{
	mActualTimeRateSource = source;
	mBoostActualTimeRateSourceConnection = connectObservableValue(source, [&](const double& oldValue, const double& newValue) {
		updateTimeRateLabel();
		});
}

void TimeControlWidget::updateTimeRateLabel()
{
	QString text;
	
	if (mRequestedTimeRateSource && mActualTimeRateSource)
	{
		text = QString::number(mRequestedTimeRateSource->get(), 'g', 3) + "x";
		if (mRequestedTimeRateSource->get() != mActualTimeRateSource->get())
		{
			text += " (" + QString::number(mActualTimeRateSource->get(), 'g', 3) + "x actual)";
			mTimeRateLabel->setStyleSheet("QLabel { color : yellow; }");
		}
		else
		{
			mTimeRateLabel->setStyleSheet("");
		}
	}

	mTimeRateLabel->setText(text);
}
