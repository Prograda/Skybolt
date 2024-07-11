/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TimelineWidget.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/Range.h>
#include <QLayout>
#include <QStyle>
#include <QToolBar>
#include <QDateTime>
#include <QMouseEvent>
#include <QPainter>

using namespace skybolt;

class TimelineSlider : public QSlider
{
public:
	TimelineSlider() : QSlider(Qt::Horizontal) {}

	void setBufferedRange(const IntRangeInclusive& range)
	{
		mBufferedRange = range;
		update();
	}

protected:
	void mousePressEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton)
		{
			setValue(minimum() + ((maximum() - minimum()) * event->x()) / width());
			event->accept();
		}
		QSlider::mousePressEvent(event);
	}

	void paintEvent(QPaintEvent *ev)
	{
		int start = QStyle::sliderPositionFromValue(minimum(), maximum(), mBufferedRange.first, width());
		int end = QStyle::sliderPositionFromValue(minimum(), maximum(), mBufferedRange.last, width());
		QPainter painter(this);
		//painter.setBrush(QBrush(QColor(190,230,190))); // Light theme
		painter.setBrush(QBrush(QColor(50, 80, 50)));
		painter.setPen(Qt::NoPen);
		painter.drawRect(start, 0, end - start, height());
		QSlider::paintEvent(ev);
	}

private:
	IntRangeInclusive mBufferedRange;
};

TimelineWidget::TimelineWidget(QWidget* parent) :
	QWidget(parent),
	mSource(nullptr),
	mBufferedRange(0,0)
{
	mTime = new QLabel;
	mDuration = new QLabel;
	mSlider = new TimelineSlider();
	mSlider->setMaximum(1000);
	mSlider->setDisabled(true);

	setLayout(new QHBoxLayout);
	layout()->setMargin(0);
	layout()->addWidget(mTime);
	layout()->addWidget(mSlider);
	layout()->addWidget(mDuration);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	mTime->setText(toTimeText(0));
	mDuration->setText(toTimeText(0));
}

void TimelineWidget::setTimeSource(TimeSource* source)
{
	mSource = source;
	mSlider->setEnabled(source != nullptr);

	// Disconnect from previous source
	for (const QMetaObject::Connection& connection : mConnections)
	{
		QObject::disconnect(connection);
	}
	mConnections.clear();
	mBoostConnections.clear();

	if (mSource)
	{
		timeChanged(mSource->getTime());
		rangeChanged(mSource->getRange());

		// Connect to new source
		mConnections.push_back(connect(mSlider, &QSlider::valueChanged, [&](int valueInt) {
			if (mTimelineMode == skybolt::TimelineMode::Free)
			{
				double value = double(valueInt) / double(mSlider->maximum() - mSlider->minimum());
				value = math::lerp(mSource->getRange().start, mSource->getRange().end, value);
				mSource->setTime(value);
			}
		}));

		mBoostConnections.push_back(mSource->timeChanged.connect([this] (double time) {timeChanged(time); }));
		mBoostConnections.push_back(mSource->rangeChanged.connect([this](TimeRange range) {rangeChanged(range); }));
	}
}

void TimelineWidget::setTimelineMode(TimelineMode timelineMode)
{
	if (mTimelineMode != timelineMode)
	{
		mTimelineMode = timelineMode;
		mSlider->setEnabled(timelineMode == TimelineMode::Free);
	}
}

void TimelineWidget::setBufferedRange(const TimeRange& range)
{
	mBufferedRange = range;

	IntRangeInclusive intRange(toSliderValue(range.start), toSliderValue(range.end));
	mSlider->setBufferedRange(intRange);
}

void TimelineWidget::timeChanged(double time)
{
	mTime->setText(toTimeText(time));

	mSlider->blockSignals(true);
	mSlider->setValue(toSliderValue(time));
	mSlider->blockSignals(false);
}

void TimelineWidget::rangeChanged(const TimeRange& range)
{
	double duration = range.end - range.start;
	mDuration->setText(toTimeText(duration));

	mSlider->setValue(toSliderValue(mSource->getTime()));
	IntRangeInclusive intRange(toSliderValue(mBufferedRange.start), toSliderValue(mBufferedRange.end));
	mSlider->setBufferedRange(intRange);
}

QString TimelineWidget::toTimeText(double time)
{
	QString str = QDateTime::fromMSecsSinceEpoch(time * 1000).toUTC().toString("hh:mm:ss.zzz");
	return str;
}

int TimelineWidget::toSliderValue(double time) const
{
	const TimeRange& range = mSource->getRange();
	double duration = range.end - range.start;
	return (duration > 0) ? (time - range.start) / duration * mSlider->maximum() : 0;
}
