/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <string>

namespace skybolt {
namespace sim {

struct AttachmentParams
{
	std::string entityTemplate; //!< Type of entity to restrict attachments to. Leave empty to allow all types.
	Vector3 positionRelBody;
	Quaternion orientationRelBody;
};

class AttachmentComponent : public Component, public sim::EntityListener
{
public:
	AttachmentComponent(const AttachmentParams& params, Entity* parentObject);
	~AttachmentComponent();

	void resetTarget(Entity* target = nullptr);

	//! May return nullptr
	Entity* getTarget() const { return mTarget; }

	const std::string& getEntityTemplate() const { return mParams.entityTemplate; }

	void updatePostDynamics(TimeReal dt, TimeReal dtWallClock) override;

	void setPositionRelBody(const Vector3& positionRelBody) { mParams.positionRelBody = positionRelBody; }
	void setOrientationRelBody(const Quaternion& orientationRelBody) { mParams.orientationRelBody = orientationRelBody; }

private:
	void onDestroy(Entity* entity) override;

	void setTargetStateToParent();

private:
	AttachmentParams mParams;
	Entity* mParentObject;
	Entity* mTarget;
	EntityListener* mTargetListener;
	ParentReferenceComponentPtr mParentReference;
};

AttachmentComponentPtr getParentAttachment(const sim::Entity& entity);

} // namespace sim
} // namespace skybolt