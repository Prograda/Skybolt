#pragma once

#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltCommon/ObservableValue.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/EntityId.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <QToolBar>

class QToolButton;
class QWidget;

struct EntityCreationToolBarConfig
{
	skybolt::NonNullPtr<skybolt::sim::World> world;
	skybolt::NonNullPtr<skybolt::EntityFactory> entityFactory;
	std::function<void(skybolt::sim::Entity&)> onEntityCreated = nullptr;
	QWidget* parent = nullptr;
};

class EntityCreationToolBar : public QToolBar
{
public:
	EntityCreationToolBar(const EntityCreationToolBarConfig& config);

	void setSelectedEntity(const skybolt::sim::EntityId& entityId);

private:
	QToolButton* mDeleteButton;
	skybolt::sim::EntityId mSelectedEntityId;
};