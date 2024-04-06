/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltVis/SkyboltVisFwd.h>

#include <functional>

namespace skybolt {

//! Synchronizes the visualization state with the simulation state each update
class SimVisSystem : public sim::System
{
public:
	using SceneOriginProvider = std::function<sim::Vector3()>;

	SimVisSystem(const sim::World* world, const vis::ScenePtr& scene);
	~SimVisSystem();

	void updatePostDynamics(const System::StepArgs& args) override;

	const GeocentricToNedConverter& getCoordinateConverter() const { return *mCoordinateConverter; }

	void setSceneOriginProvider(SceneOriginProvider sceneOriginProvider) { mSceneOriginProvider = std::move(sceneOriginProvider); }

	static SceneOriginProvider sceneOriginFromPosition(const sim::Vector3& position);
	static SceneOriginProvider sceneOriginFromEntity(const sim::World* world, const sim::EntityId& entity);
	static SceneOriginProvider sceneOriginFromFirstCamera(const sim::World* world);

private:
	const sim::World* mWorld;
	vis::ScenePtr mScene;
	SceneOriginProvider mSceneOriginProvider;
	std::unique_ptr<GeocentricToNedConverter> mCoordinateConverter;
};

} // namespace skybolt