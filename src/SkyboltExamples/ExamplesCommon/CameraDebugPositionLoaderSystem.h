/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltVis/SkyboltVisFwd.h>
#include <SkyboltCommon/Event.h>

#include <osg/ref_ptr>

namespace skybolt {

class DebugPositionLoaderSystem : public EventListener, public sim::System
{
public:
	DebugPositionLoaderSystem(InputPlatform* inputPlatform, sim::Node* node);

	~DebugPositionLoaderSystem();

private:
	void onEvent(const Event& event) override;

private:
	InputPlatform* mInputPlatform;
	sim::Node* mNode;
};

} // namespace skybolt