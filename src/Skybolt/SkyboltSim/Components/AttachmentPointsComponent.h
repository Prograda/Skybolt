/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <string>

namespace skybolt {
namespace sim {

struct AttachmentPoint
{
	Vector3 positionRelBody;
	Quaternion orientationRelBody;
};

class AttachmentPointsComponent : public Component
{
public:
	std::map<std::string, AttachmentPointPtr> attachmentPoints;
};

void addAttachmentPoint(Entity& entity, const std::string& name, const AttachmentPointPtr& point);

//@return nullptr if not found
AttachmentPointPtr findAttachmentPoint(const Entity& entity, const std::string& name);

Vector3 calcAttachmentPointPosition(const Entity& entity, const AttachmentPoint& point);
Quaternion calcAttachmentPointOrientation(const Entity& entity, const AttachmentPoint& point);

} // namespace sim
} // namespace skybolt