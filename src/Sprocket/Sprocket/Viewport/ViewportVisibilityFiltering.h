/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>
#include <memory>

class QMenu;
class QString;
class QWidget;

enum class SelectedObjectVisibilityFilter
{
	Off, //!< No objects included in visibility set
	Selected, //!< Selected object included in visibility set
	On //!< All objects included in visibility set
};

using SelectedObjectVisibilityFilterPtr = std::shared_ptr<SelectedObjectVisibilityFilter>;

QMenu* createVisibilityFilterMenu(const QString& name, const SelectedObjectVisibilityFilterPtr& filter, QWidget* parent = nullptr);

skybolt::EntityVisibilityPredicate createSelectedEntityVisibilityPredicate(const SelectedObjectVisibilityFilterPtr& filter, const ScenarioSelectionModel* selectionModel);

skybolt::EntityVisibilityPredicate createLineOfSightVisibilityPredicate(ViewportWidget* viewportWidget, skybolt::sim::World* world);

skybolt::EntityVisibilityPredicate predicateAnd(const skybolt::EntityVisibilityPredicate& a, const skybolt::EntityVisibilityPredicate& b);

skybolt::EntityVisibilityPredicate predicateOr(const skybolt::EntityVisibilityPredicate& a, const skybolt::EntityVisibilityPredicate& b);

skybolt::EntityVisibilityPredicate createSelectedEntityVisibilityPredicateAndAddSubMenu(QMenu& parent, const QString& name, const ScenarioSelectionModel* selectionModel);

void addVisibilityLayerSubMenus(QMenu& parent, const skybolt::EntityVisibilityPredicate& basePredicate, const std::map<std::string, skybolt::EntityVisibilityPredicateSetter>& layerMap, const ScenarioSelectionModel* selectionModel);