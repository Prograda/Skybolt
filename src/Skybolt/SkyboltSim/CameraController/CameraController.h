/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Entity.h"
#include "SkyboltSim/SkyboltSimFwd.h"

namespace skybolt {
namespace sim {

	class CameraController : public EntityListener
	{
	public:
		struct Input
		{
			static Input zero()
			{
				return {0,0,0,0,0,false};
			}

			float forwardSpeed; //!< Range [-1, 1]
			float rightSpeed; //!< Range [-1, 1]
			float panSpeed; //!< Range [-1, 1]
			float tiltSpeed; //!< Range [-1, 1]
			float zoomSpeed; //!< Range [-1, 1]
			bool modifierPressed;
		};

		CameraController(Entity* camera);
		virtual ~CameraController();

		virtual void updatePostDynamicsSubstep(TimeReal dtSubstep) {}
		virtual void update(float dt) {}
		virtual void setInput(const Input& input) {}

		virtual Entity* getTarget() const { return mTarget; }
		virtual void setTarget(Entity* target);

		virtual void setActive(bool active) { mActive = active; }

	private:
		void onDestroy(Entity* entity) override;

	protected:
		sim::Node* mNodeComponent;
		CameraComponent* mCameraComponent;
		Entity* mTarget = nullptr;
		bool mActive = false;
	};

} // namespace sim
} // namespace skybolt