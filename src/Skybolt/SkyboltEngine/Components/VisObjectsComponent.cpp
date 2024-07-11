/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisObjectsComponent.h"
#include "SkyboltVis/Scene.h"

namespace skybolt {

VisObjectsComponent::~VisObjectsComponent()
{
	for (const vis::VisObjectPtr& object : objects)
	{
		scene->removeObject(object);
	}
}

void VisObjectsComponent::addObject(const vis::VisObjectPtr& object, bool addToScene)
{
	if (addToScene)
	{
		scene->addObject(object);
	}
	objects.push_back(object);
}

} // namespace skybolt