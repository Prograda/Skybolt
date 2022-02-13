/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimVisBinding.h"
#include "SimVisObjectsReflector.h"
#include <SkyboltSim/World.h>
#include <SkyboltVis/SkyboltVisFwd.h>
#include <osg/Switch>
#include <osg/MatrixTransform>

namespace skybolt {

class VisNameLabels : public SimVisObjectsReflector<osg::MatrixTransform*>, public SimVisBinding
{
public:
	VisNameLabels(sim::World* world, osg::Group* parent, const vis::ShaderPrograms& programs);
	~VisNameLabels();

	void syncVis(const GeocentricToNedConverter& converter) override;

	std::optional<osg::MatrixTransform*> createObject(const sim::EntityPtr& entity) override;

	void destroyObject(osg::MatrixTransform* const& object) override {}

	osg::Node* getNode(osg::MatrixTransform* const& object) const override
	{
		return object;
	}
};

} // namespace skybolt