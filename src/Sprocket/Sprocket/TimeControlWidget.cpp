/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TimeControlWidget.h"
#include "Icon/SprocketIcons.h"
#include <QLayout>
#include <QStyle>
#include <QToolBar>

using namespace skybolt;

TimeControlWidget::TimeControlWidget(QWidget* parent) :
	QWidget(parent),
	mSource(nullptr)
{
	mPlayAction = new QAction(getSprocketIcon(SprocketIcon::Play), tr("Play"), this);
	mPlayAction->setShortcut(tr("Ctrl+P"));

	mForwardAction = new QAction(getSprocketIcon(SprocketIcon::FastForward), tr("Forward"), this);
	mForwardAction->setShortcut(tr("Ctrl+F"));
	
	mBackwardAction = new QAction(getSprocketIcon(SprocketIcon::FastRewind), tr("Backward"), this);
	mBackwardAction->setShortcut(tr("Ctrl+B"));

	QToolBar *bar = new QToolBar;
	bar->addAction(mBackwardAction);
	bar->addAction(mPlayAction);
	bar->addAction(mForwardAction);

	setLayout(new QHBoxLayout);
	layout()->setMargin(0);
	layout()->addWidget(bar);

	setEnabled(false);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void TimeControlWidget::setTimeSource(TimeSource* source)
{
	mSource = source;
	setEnabled(source != nullptr);

	// Disconnect from previous source
	for (const QMetaObject::Connection& connection : mConnections)
	{
		QObject::disconnect(connection);
	}
	mConnections.clear();

	if (mSource)
	{
		// Connect to new source
		mConnections.push_back(connect(mPlayAction, &QAction::triggered, [&]()
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

		mConnections.push_back(connect(mForwardAction, &QAction::triggered, [&]() { mSource->setTime(mSource->getRange().end); }));
		mConnections.push_back(connect(mBackwardAction, &QAction::triggered, [&]() { mSource->setTime(mSource->getRange().start); }));

		mBoostConnections.push_back(mSource->stateChanged.connect([&](const TimeSource::State& state) {
			switch (state)
			{
			case TimeSource::StatePlaying:
				mPlayAction->setIcon(getSprocketIcon(SprocketIcon::Pause));
				break;
			case TimeSource::StateStopped:
				mPlayAction->setIcon(getSprocketIcon(SprocketIcon::Play));
				break;
			default:
				break;
			}
		}));
	}
}
