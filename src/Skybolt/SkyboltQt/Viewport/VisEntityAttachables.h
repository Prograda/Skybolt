/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SimVisBinding/SimVisBinding.h>
#include <SkyboltSim/Entity.h>
#include <osg/Group>
#include <osg/MatrixTransform>

namespace skybolt {

class VisEntityAttachables : public osg::Group
{
public:
	~VisEntityAttachables() override = default;

	void syncVis(const GeocentricToNedConverter& converter);

	void setEntities(std::set<sim::Entity*> entities);

protected:
	virtual osg::ref_ptr<osg::Node> createNode(sim::Entity* entity) const = 0;

private:
	std::map<sim::Entity*, osg::ref_ptr<osg::MatrixTransform>> mEntityTransforms;
};

} // namespace skybolt