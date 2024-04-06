#include "EntityInputSystem.h"
#include <SkyboltEngine/Input/LogicalAxis.h>
#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Entity.h>

using namespace skybolt;

EntityInputSystem::EntityInputSystem(const std::vector<LogicalAxisPtr>& axes) :
	mAxes(axes)
{
}

void EntityInputSystem::updateState()
{
	if (mEntity)
	{
		auto inputs = mEntity->getFirstComponent<sim::ControlInputsComponent>();
		if (inputs)
		{
			inputs->setIfPresent("stick", glm::vec2(mAxes[1]->getState(), -mAxes[0]->getState()));
			inputs->setIfPresent("collective", mAxes[2]->getState());
			inputs->setIfPresent("pedal", mAxes[3]->getState());
		}
	}
}

void EntityInputSystem::setEntity(const sim::EntityPtr& entity)
{
	mEntity = entity;
}
