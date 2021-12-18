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

	void updatePreDynamics(const sim::System::StepArgs& args) override;

	void setEntity(const sim::EntityPtr& entity);

private:
	sim::EntityPtr mEntity;
	std::vector<LogicalAxisPtr> mAxes;
};

} // namespace skybolt