/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/Scenario/ScenarioObject.h"

#include <QObject>
#include <memory>

class SceneSelectionModel : public QObject
{
	Q_OBJECT
public:
	SceneSelectionModel(QObject* parent = nullptr);

	void selectItem(const ScenarioObjectPtr& item);
	ScenarioObjectPtr getSelectedItem() const { return mSelectedItem; }

public:
	Q_SIGNAL void selectionChanged(const ScenarioObjectPtr& selected, const ScenarioObjectPtr& deselected);

private:
	ScenarioObjectPtr mSelectedItem;
};
