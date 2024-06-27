/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include <SkyboltCommon/ObservableValue.h>
#include <SkyboltEngine/SimVisBinding/EntityVisibilityFilterable.h>

#include <QPointer>
#include <memory>

class QMenu;
class QString;
class QWidget;

enum class ObjectVisibilityFilterState
{
	Off, //!< No objects included in visibility set
	Selected, //!< Selected object included in visibility set
	On //!< All objects included in visibility set
};

using ObjectVisibilityFilter = skybolt::ObservableValue<ObjectVisibilityFilterState>;
using ObjectVisibilityFilterPtr = std::shared_ptr<ObjectVisibilityFilter>;

QMenu* createVisibilityFilterMenu(const QString& name, const ObjectVisibilityFilterPtr& filter, QWidget* parent = nullptr);
QMenu* createVisibilityFilterSubMenu(QMenu& parent, const QString& name, const ObjectVisibilityFilterPtr& filter);

skybolt::EntityVisibilityPredicate createSelectedEntityVisibilityPredicate(const ObjectVisibilityFilterPtr& filter, const ScenarioSelectionModel* selectionModel);

skybolt::EntityVisibilityPredicate createLineOfSightVisibilityPredicate(QPointer<ViewportWidget> viewportWidget, skybolt::sim::World* world);

skybolt::EntityVisibilityPredicate predicateAnd(const skybolt::EntityVisibilityPredicate& a, const skybolt::EntityVisibilityPredicate& b);
skybolt::EntityVisibilityPredicate predicateAnd(const std::vector<skybolt::EntityVisibilityPredicate>& v);

skybolt::EntityVisibilityPredicate predicateOr(const skybolt::EntityVisibilityPredicate& a, const skybolt::EntityVisibilityPredicate& b);

skybolt::EntityVisibilityPredicate createSelectedEntityVisibilityPredicateAndAddSubMenu(QMenu& parent, const std::string& name, const ScenarioSelectionModel* selectionModel);

ObjectVisibilityFilterPtr addVisibilityLayerSubMenu(QMenu& parent, const skybolt::EntityVisibilityPredicate& basePredicate, const std::string& name, const skybolt::EntityVisibilityPredicateSetter& predicateSetter, const ScenarioSelectionModel* selectionModel);
void addVisibilityLayerSubMenus(QMenu& parent, const skybolt::EntityVisibilityPredicate& basePredicate, const std::map<std::string, skybolt::EntityVisibilityPredicateSetter>& layerMap, const ScenarioSelectionModel* selectionModel);