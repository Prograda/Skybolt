/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/Scenario/ScenarioObject.h"

#include <QObject>
#include <memory>

using SelectedScenarioObjects = std::vector<ScenarioObjectPtr>;

inline ScenarioObjectPtr getFirstSelectedScenarioObject(const SelectedScenarioObjects& objects)
{
	return objects.empty() ? nullptr : objects.front();
}

template <typename T>
std::shared_ptr<T> getFirstSelectedScenarioObjectOfType(const SelectedScenarioObjects& objects)
{
	if (auto object = getFirstSelectedScenarioObject(objects); object)
	{
		return std::dynamic_pointer_cast<T>(object);
	}
	return nullptr;
}

class ScenarioSelectionModel : public QObject
{
	Q_OBJECT
public:
	ScenarioSelectionModel(QObject* parent = nullptr);

	void setSelectedItems(const SelectedScenarioObjects& item);
	SelectedScenarioObjects getSelectedItems() const { return mSelectedItems; }

public:
	Q_SIGNAL void selectionChanged(const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected);

private:
	SelectedScenarioObjects mSelectedItems;
};
