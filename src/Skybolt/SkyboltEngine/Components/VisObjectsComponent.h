/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltVis/VisObject.h>
#include <memory>
#include <vector>

namespace skybolt {

class VisObjectsComponent : public sim::Component
{
public:
	VisObjectsComponent(vis::Scene* scene) : scene(scene) {}
	~VisObjectsComponent();

	void addObject(const vis::VisObjectPtr& object, bool addToScene = true);

	const std::vector<vis::VisObjectPtr>& getObjects() const { return objects; }

private:
	std::vector<vis::VisObjectPtr> objects;
	vis::Scene* scene;
};

template <class T>
std::shared_ptr<T> getFirstVisObject(const sim::Entity& entity)
{
	VisObjectsComponent* visObjects = entity.getFirstComponent<VisObjectsComponent>().get();
	if (visObjects)
	{
		for (const vis::VisObjectPtr& visObject : visObjects->getObjects())
		{
			auto derivedObject = std::dynamic_pointer_cast<T>(visObject);
			if (derivedObject)
			{
				return derivedObject;
			}
		}
	}
	return nullptr;
}

} // namespace skybolt