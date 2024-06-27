/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportVisibilityFiltering.h"
#include "SkyboltQt/Scenario/EntityObjectType.h"
#include "SkyboltQt/Scenario/ScenarioSelectionModel.h"
#include "SkyboltQt/Widgets/ViewportWidget.h"

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

QMenu* createVisibilityFilterMenu(const QString& name, const ObjectVisibilityFilterPtr& filter, QWidget* parent)
{
	auto menu = new QMenu(name, parent);
	QActionGroup* alignmentGroup = new QActionGroup(menu);

	QAction* offAction = addCheckedAction(*menu, "Hide All", [=](bool checked) {
		filter->set(ObjectVisibilityFilterState::Off);
	});

	QAction* selectedAction = addCheckedAction(*menu, "Show Selected", [=](bool checked) {
		filter->set(ObjectVisibilityFilterState::Selected);
	});

	QAction* onAction = addCheckedAction(*menu, "Show All", [=](bool checked) {
		filter->set(ObjectVisibilityFilterState::On);
	});

	alignmentGroup->addAction(offAction);
	alignmentGroup->addAction(selectedAction);
	alignmentGroup->addAction(onAction);

	auto applyState = [=] (const ObjectVisibilityFilterState& oldValue, const ObjectVisibilityFilterState& newValue) {
		switch(newValue)
		{
			case ObjectVisibilityFilterState::Off:
				offAction->setChecked(true);
				break;
			case ObjectVisibilityFilterState::Selected:
				selectedAction->setChecked(true);
				break;
			case ObjectVisibilityFilterState::On:
				onAction->setChecked(true);
				break;
		};
	};

	filter->valueChanged.connect(applyState);
	applyState(filter->get(), filter->get());

	return menu;
}

QMenu* createVisibilityFilterSubMenu(QMenu& parent, const QString& name, const std::shared_ptr<ObjectVisibilityFilter>& filter)
{
	QMenu* menu = createVisibilityFilterMenu(name, filter);
	parent.addMenu(menu);
	return menu;
}

EntityVisibilityPredicate createSelectedEntityVisibilityPredicate(const ObjectVisibilityFilterPtr& filter, const ScenarioSelectionModel* selectionModel)
{
	return [filter, selectionModel] (const sim::Entity& entity) {
		switch(filter->get())
		{
			case ObjectVisibilityFilterState::Off:
				return false;
			case ObjectVisibilityFilterState::Selected:
				for (const auto& object : selectionModel->getSelectedItems())
				{
					if (auto entityObject = dynamic_cast<EntityObject*>(object.get()); entityObject)
					{
						return (entityObject->data == entity.getId());
					}
				}
				return false;
			case ObjectVisibilityFilterState::On:
				return true;
		};
		assert(!"Should not get here");
		return false;
	};
}

EntityVisibilityPredicate createLineOfSightVisibilityPredicate(QPointer<ViewportWidget> viewportWidget, sim::World* world)
{
	return [viewportWidget, world] (const sim::Entity& entity) {
		if (!viewportWidget)
		{
			return false;
		}

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

skybolt::EntityVisibilityPredicate predicateAnd(const std::vector<skybolt::EntityVisibilityPredicate>& v)
{
	return [=] (const sim::Entity& entity) {
		for (const auto& i : v)
		{
			if (!i(entity))
			{
				return false;
			}
		}
		return true;
	};
}

skybolt::EntityVisibilityPredicate predicateOr(const skybolt::EntityVisibilityPredicate& a, const skybolt::EntityVisibilityPredicate& b)
{
	return [=] (const sim::Entity& entity) {
		return a(entity) || b(entity);
	};
}

EntityVisibilityPredicate createSelectedEntityVisibilityPredicateAndAddSubMenu(QMenu& parent, const std::string& name, const ScenarioSelectionModel* selectionModel)
{
	auto filter = std::make_shared<ObjectVisibilityFilter>(ObjectVisibilityFilterState::On);
	createVisibilityFilterSubMenu(parent, QString::fromStdString(name), filter);
	return createSelectedEntityVisibilityPredicate(filter, selectionModel);
}

ObjectVisibilityFilterPtr addVisibilityLayerSubMenu(QMenu& parent, const skybolt::EntityVisibilityPredicate& basePredicate, const std::string& name, const skybolt::EntityVisibilityPredicateSetter& predicateSetter, const ScenarioSelectionModel* selectionModel)
{
	auto filter = std::make_shared<ObjectVisibilityFilter>(ObjectVisibilityFilterState::On);
	QMenu* menu = createVisibilityFilterSubMenu(parent, QString::fromStdString(name), filter);

	EntityVisibilityPredicate predicate = createSelectedEntityVisibilityPredicate(filter, selectionModel);
	predicateSetter(predicateAnd(predicate, basePredicate));
	
	return filter;
}

void addVisibilityLayerSubMenus(QMenu& parent, const EntityVisibilityPredicate& basePredicate, const std::map<std::string, EntityVisibilityPredicateSetter>& layerMap, const ScenarioSelectionModel* selectionModel)
{
	for (const auto& layer : layerMap)
	{
		addVisibilityLayerSubMenu(parent, basePredicate, layer.first, layer.second, selectionModel);
	}
}