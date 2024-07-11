/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ForcesVisBinding.h"
#include "GeocentricToNedConverter.h"

#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/Renderable/Arrows.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <assert.h>

namespace skybolt {

constexpr double forceScaleMetersPerNewton = 0.0001;

ForcesVisBinding::ForcesVisBinding(sim::World* world, const vis::ArrowsPtr& arrows) :
	mWorld(world),
	mArrows(arrows),
	mEntityVisibilityPredicate(visibilityOn)
{
	assert(mWorld);
	assert(mArrows);
}

void ForcesVisBinding::syncVis(const GeocentricToNedConverter& converter)
{
	std::vector<vis::Vec3Segment> segments;

	const auto& entities = mWorld->getEntities();
	for (const sim::EntityPtr& entity : entities)
	{
		if (mEntityVisibilityPredicate(*entity))
		{
			if (entity->isDynamicsEnabled())
			{
				const sim::Node* node = entity->getFirstComponent<sim::Node>().get();
				const sim::DynamicBodyComponent* body = entity->getFirstComponent<sim::DynamicBodyComponent>().get();
				if (node && body)
				{
					const std::vector<sim::AppliedForce>& forces = body->getForces();

					for (const sim::AppliedForce& force : forces)
					{
						sim::Vector3 start = node->getPosition() + force.positionRelBody;

						vis::Vec3Segment segment;
						segment.start = converter.convertPosition(start);
						segment.end = converter.convertPosition(start + force.force * forceScaleMetersPerNewton);

						segments.push_back(segment);
					}
				}
			}
		}
	}

	mArrows->setSegments(segments);
}

std::shared_ptr<ForcesVisBinding> createForcesVisBinding(sim::World* world, const vis::ScenePtr& scene, const vis::ShaderPrograms& programs)
{
	vis::Arrows::Params params;
	params.program = programs.getRequiredProgram("unlitColored");

	auto arrows = std::make_shared<vis::Arrows>(params);
	scene->addObject(arrows);
	return std::shared_ptr<ForcesVisBinding>(new ForcesVisBinding(world, arrows), [scene, arrows] (ForcesVisBinding* binding) {
		scene->removeObject(arrows);
		delete binding;
	});
}

} // namespace skybolt