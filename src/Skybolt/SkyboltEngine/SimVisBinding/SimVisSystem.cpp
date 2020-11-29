/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimVisSystem.h"
#include "EngineRoot.h"
#include "SimVisBinding/GeocentricToNedConverter.h"
#include "SimVisBinding/SimVisBinding.h"
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <assert.h>

namespace skybolt {

using namespace sim;

SimVisSystem::SimVisSystem(const World* world, const vis::ScenePtr& scene) :
	mSceneOriginProvider(sceneOriginFromFirstCamera(world)),
	mWorld(world),
	mScene(scene),
	mCoordinateConverter(std::make_unique<GeocentricToNedConverter>())
{
	assert(mWorld);
	assert(mScene);
}

SimVisSystem::~SimVisSystem()
{
}

void SimVisSystem::updatePostDynamics(const System::StepArgs& args)
{
	// Calculate camera movement since last frame and apply offset to scene noise
	Vector3 cameraPosition = mSceneOriginProvider();

	osg::Vec3f cameraNedMovement = mCoordinateConverter->convertPosition(cameraPosition);
	mScene->translateNoiseOrigin(-cameraNedMovement);
	mCoordinateConverter->setOrigin(cameraPosition);

	syncVis(*mWorld, *mCoordinateConverter);
}

SimVisSystem::SceneOriginProvider SimVisSystem::sceneOriginFromPosition(const sim::Vector3& position)
{
	return [=] { return position; };
}

SimVisSystem::SceneOriginProvider SimVisSystem::sceneOriginFromEntity(const sim::EntityPtr& entity)
{
	return [=]{
		auto position = getPosition(*entity);
		{
			return *position;
		}
		return sim::Vector3(0, 0, 0);
	};
}

class SceneOriginFromFirstCamera : public Entity::Listener
{
public:
	SceneOriginFromFirstCamera(const sim::World* world) :
		mWorld(world)
	{
		assert(mWorld);
	}

	~SceneOriginFromFirstCamera()
	{
		if (mCamera)
		{
			mCamera->removeListener(this);
		}
	}

	sim::Vector3 operator()() {
		if (!mCamera)
		{
			for (const auto& entity : mWorld->getEntities())
			{
				if (entity->getFirstComponent<sim::CameraComponent>())
				{
					mCamera = entity.get();
					mCamera->addListener(this);
					break;
				}
			}
		}
		if (mCamera)
		{
			auto position = getPosition(*mCamera);
			{
				return *position;
			}
		}
		return sim::Vector3(0, 0, 0);
	}

	void onDestroy(Entity* entity) override
	{
		assert(entity == mCamera);
		mCamera = nullptr;
	}

private:
	const sim::World* mWorld;
	sim::Entity* mCamera = nullptr;
};

SimVisSystem::SceneOriginProvider SimVisSystem::sceneOriginFromFirstCamera(const sim::World* world)
{
	return SceneOriginFromFirstCamera(world);
}

} // namespace skybolt