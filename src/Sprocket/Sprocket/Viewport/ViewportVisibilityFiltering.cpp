/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportVisibilityFiltering.h"
#include "Sprocket/Scenario/EntityObjectType.h"
#include "Sprocket/Scenario/ScenarioSelectionModel.h"
#include "Sprocket/Widgets/ViewportWidget.h"

#include <SkyboltCommon/Math/IntersectionUtility.h>
#include <SkyboltSim/WorldUtil.h>
#include <SkyboltSim/Components/PlanetComponent.h>

#include <QMenu>

using namespace skybolt;

static QAction* addCheckedAction(QMenu& menu, const QString& text, std::function<void(bool checked)> fn)
{
	QAction* action = menu.addAction(text, fn);
	action->setCheckable(true);
	return action;
}

QMenu* createVisibilityFilterMenu(const QString& name, const SelectedObjectVisibilityFilterPtr& filter, QWidget* parent)
{
	auto menu = new QMenu(name, parent);
	QActionGroup* alignmentGroup = new QActionGroup(menu);

	QAction* offAction = addCheckedAction(*menu, "Hide All", [=](bool checked) {
		(*filter) = SelectedObjectVisibilityFilter::Off;
	});

	QAction* selectedAction = addCheckedAction(*menu, "Show Selected", [=](bool checked) {
		(*filter) = SelectedObjectVisibilityFilter::Selected;
	});

	QAction* onAction = addCheckedAction(*menu, "Show All", [=](bool checked) {
		(*filter) = SelectedObjectVisibilityFilter::On;
	});

	alignmentGroup->addAction(offAction);
	alignmentGroup->addAction(selectedAction);
	alignmentGroup->addAction(onAction);

	switch(*filter)
	{
		case SelectedObjectVisibilityFilter::Off:
			offAction->setChecked(true);
			break;
		case SelectedObjectVisibilityFilter::Selected:
			selectedAction->setChecked(true);
			break;
		case SelectedObjectVisibilityFilter::On:
			onAction->setChecked(true);
			break;
	};

	return menu;
}

EntityVisibilityPredicate createSelectedEntityVisibilityPredicate(const SelectedObjectVisibilityFilterPtr& filter, const ScenarioSelectionModel* selectionModel)
{
	return [filter, selectionModel] (const sim::Entity& entity) {
		switch(*filter)
		{
			case SelectedObjectVisibilityFilter::Off:
				return false;
			case SelectedObjectVisibilityFilter::Selected:
				for (const auto& object : selectionModel->getSelectedItems())
				{
					if (auto entityObject = dynamic_cast<EntityObject*>(object.get()); entityObject)
					{
						return (entityObject->data == entity.getId());
					}
				}
				return false;
			case SelectedObjectVisibilityFilter::On:
				return true;
		};
		assert(!"Should not get here");
		return false;
	};
}

EntityVisibilityPredicate createLineOfSightVisibilityPredicate(ViewportWidget* viewportWidget, sim::World* world)
{
	return [viewportWidget, world] (const sim::Entity& entity) {
		if (const auto& entityPosition = getPosition(entity); entityPosition)
		{
			if (sim::Entity* camera = viewportWidget->getCamera(); camera)
			{
				sim::Vector3 cameraPosition = *getPosition(*camera);
				if (sim::Entity* planet = sim::findNearestEntityWithComponent<sim::PlanetComponent>(world->getEntities(), cameraPosition).get(); planet)
				{
					const sim::PlanetComponent& planetComponent = *planet->getFirstComponent<sim::PlanetComponent>();

					sim::Vector3 planetPosition = *getPosition(*planet);
					sim::Vector3 camToEntityDir = *entityPosition - cameraPosition;
					double length = glm::length(camToEntityDir);
					camToEntityDir /= length;

					double effectivePlanetRadius = std::max(0.0, planetComponent.radius * 0.999); // use a slightly smaller radius for robustness
					return !intersectRaySegmentSphere(cameraPosition, camToEntityDir, length, planetPosition, effectivePlanetRadius);
				}
			}
		}
		return false;
	};
}

skybolt::EntityVisibilityPredicate predicateAnd(const skybolt::EntityVisibilityPredicate& a, const skybolt::EntityVisibilityPredicate& b)
{
	return [=] (const sim::Entity& entity) {
		return a(entity) && b(entity);
	};
}

skybolt::EntityVisibilityPredicate predicateOr(const skybolt::EntityVisibilityPredicate& a, const skybolt::EntityVisibilityPredicate& b)
{
	return [=] (const sim::Entity& entity) {
		return a(entity) || b(entity);
	};
}

EntityVisibilityPredicate createSelectedEntityVisibilityPredicateAndAddSubMenu(QMenu& parent, const QString& name, const ScenarioSelectionModel* selectionModel)
{
	auto visibilityFilter = std::make_shared<SelectedObjectVisibilityFilter>();
	(*visibilityFilter) = SelectedObjectVisibilityFilter::On;
	EntityVisibilityPredicate predicate = createSelectedEntityVisibilityPredicate(visibilityFilter, selectionModel);
	QMenu* menu = createVisibilityFilterMenu(name, visibilityFilter);
	parent.addMenu(menu);
	return predicate;
}

void addVisibilityLayerSubMenus(QMenu& parent, const EntityVisibilityPredicate& basePredicate, const std::map<std::string, EntityVisibilityPredicateSetter>& layerMap, const ScenarioSelectionModel* selectionModel)
{
	for (const auto& layer : layerMap)
	{
		QString name = QString::fromStdString(layer.first);
		EntityVisibilityPredicate predicate = createSelectedEntityVisibilityPredicateAndAddSubMenu(parent, name, selectionModel);
		layer.second(predicateAnd(predicate, basePredicate));
	}
}