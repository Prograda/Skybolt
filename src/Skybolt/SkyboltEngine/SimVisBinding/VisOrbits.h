/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimVisBinding.h"
#include "SimVisObjectsReflector.h"
#include <SkyboltVis/Renderable/Polyline.h>
#include <osg/Switch>
#include <osg/MatrixTransform>

namespace skybolt {

class VisOrbits : public SimVisObjectsReflector<vis::PolylinePtr>, public SimVisBinding
{
public:
	VisOrbits(sim::World* world, osg::Group* parent, const vis::Polyline::Params& params, JulianDateProvider julianDateProvider);
	~VisOrbits();

	void syncVis(const GeocentricToNedConverter& converter) override;

private:
	std::optional<vis::PolylinePtr> createObject(const sim::EntityPtr& entity) override;

	void destroyObject(const vis::PolylinePtr& object) override {}

	osg::Node* getNode(const vis::PolylinePtr& object) const override
	{
		return object->_getNode();
	}

private:
	vis::Polyline::Params mParams;
	JulianDateProvider mJulianDateProvider;
};

} // namespace skybolt