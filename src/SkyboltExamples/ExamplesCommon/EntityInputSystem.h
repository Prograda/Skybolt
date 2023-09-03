#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/System/System.h>

#include <vector>

namespace skybolt {

class EntityInputSystem : public sim::System
{
public:
	EntityInputSystem(const std::vector<LogicalAxisPtr>& axes);

	void setEntity(const sim::EntityPtr& entity);

public:
	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::BeginStateUpdate, updateState)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updateState();

private:
	sim::EntityPtr mEntity;
	std::vector<LogicalAxisPtr> mAxes;
};

} // namespace skybolt