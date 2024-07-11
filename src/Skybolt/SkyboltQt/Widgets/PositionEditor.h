/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SkyboltSimFwd.h>
#include <QWidget>

class QStackedWidget;

class PositionEditor : public QWidget
{
	Q_OBJECT
public:
	PositionEditor(QWidget* parent = nullptr);

	void setPosition(const skybolt::sim::Position& position);

	skybolt::sim::PositionPtr getPosition();

	Q_SIGNAL void valueChanged(const skybolt::sim::Position& position);

private:
	void addEditor(class PositionEditorImpl*);
	class PositionEditorImpl* getCurrentEditor() const;

private:
	QStackedWidget* mStackedWidget;
};
