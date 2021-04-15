/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraDebugPositionLoaderSystem.h"

#include <SkyboltEngine/Input/InputPlatform.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltVis/SkyboltVisFwd.h>

#include <osg/ref_ptr>
#include <assert.h>
#include <fstream>
#include <istream>
#include <ostream>

namespace skybolt {

DebugPositionLoaderSystem::DebugPositionLoaderSystem(InputPlatform* inputPlatform, sim::Node* node) :
	mInputPlatform(inputPlatform),
	mNode(node)
{
	mInputPlatform->getEventEmitter()->addEventListener<KeyEvent>(this);
	assert(mNode);
}

DebugPositionLoaderSystem::~DebugPositionLoaderSystem()
{
	mInputPlatform->getEventEmitter()->removeEventListener(this);
}

void DebugPositionLoaderSystem::onEvent(const Event& event)
{
	if (const auto& keyEvent = dynamic_cast<const KeyEvent*>(&event))
	{
		if (keyEvent->code == KC_P)
		{
			sim::Vector3 pos = mNode->getPosition();

			std::ofstream file("camera.txt");
			file << pos.x << " " << pos.y << " " << pos.z << std::endl;
			file.close();

			return;
		}
		else if (keyEvent->code == KC_O)
		{
			sim::Vector3 pos;
			double v;

			std::ifstream file("camera.txt");
			file >> v;
			pos.x = v;
			file >> v;
			pos.y = v;
			file >> v;
			pos.z = v;
			file.close();

			mNode->setPosition(pos);

			return;
		}
	}
}

} // namespace skybolt