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
#include <SkyboltSim/Components/PlanetComponent.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/WorldUtil.h>
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
	Vector3 origin = mSceneOriginProvider();

	// Get nearest planet
	sim::Entity* planet = findNearestEntityWithComponent<sim::PlanetComponent>(mWorld->getEntities(), origin);
	std::optional<GeocentricToNedConverter::PlanetPose> planetPose;
	if (planet)
	{
		GeocentricToNedConverter::PlanetPose p;
		p.position = *getPosition(*planet);
		p.orientation = *getOrientation(*planet);
		planetPose = p;
	}

	// Calculate planet-space origin movement since last frame and apply offset to scene noise so that the
	// noise appears fixed relative to the planet, even though the origin and planet may have moved.
	auto prevPlanetPose = mCoordinateConverter->getPlanetPose();
	if (planetPose && prevPlanetPose)
	{
		sim::Vector3 originPlanetSpace = glm::inverse(planetPose->orientation) * (origin - planetPose->position);
		sim::Vector3 originWorldSpaceUsingPrevPlanetPose = (prevPlanetPose->orientation * originPlanetSpace) + prevPlanetPose->position;

		// Calculate the origin's translation since the previous frame by querying the position of the new origin
		// relative to the previous frame's origin and planet pose.
		osg::Vec3f originTranslation = mCoordinateConverter->convertPosition(originWorldSpaceUsingPrevPlanetPose);
		mScene->translateNoiseOrigin(-originTranslation);
	}

	// Update viz origin
	mCoordinateConverter->setOrigin(origin, planetPose);

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