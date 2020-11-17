/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SimVisSystem.h"
#include "EngineRoot.h"
#include "SimVisBinding/GeocentricToNedConverter.h"
#include "SimVisBinding/SimVisBinding.h"
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Entity.h>
#include <assert.h>

namespace skybolt {

using namespace sim;

SimVisSystem::SimVisSystem(EngineRoot* engineRoot, const EntityPtr& simCamera) :
	mEngineRoot(engineRoot),
	mSimCamera(simCamera),
	mCoordinateConverter(std::make_unique<GeocentricToNedConverter>())
{
	assert(mEngineRoot);
	assert(mSimCamera);
}

SimVisSystem::~SimVisSystem()
{
}

void SimVisSystem::updatePostDynamics(const System::StepArgs& args)
{
	mEngineRoot->scenario.timeSource.setTime(mEngineRoot->scenario.timeSource.getTime() + args.dtWallClock);

	// Calculate camera movement since last frame and apply offset to scene noise
	Vector3 cameraPosition = mSimCamera->getFirstComponent<Node>()->getPosition();

	osg::Vec3f cameraNedMovement = mCoordinateConverter->convertPosition(*getPosition(*mSimCamera));
	mEngineRoot->scene->translateNoiseOrigin(-cameraNedMovement);
	mCoordinateConverter->setOrigin(cameraPosition);

	syncVis(*mEngineRoot->simWorld, *mCoordinateConverter);
}

} // namespace skybolt