/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisEntityAttachables.h"
#include <SkyboltEngine/SimVisBinding/GeocentricToNedConverter.h>
#include <SkyboltVis/VisibilityCategory.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <SkyboltCommon/ContainerUtility.h>
#include <osg/Depth>
#include <osg/Geode>
#include <osgText/Text>

namespace skybolt {

using namespace sim;

void VisEntityAttachables::syncVis(const GeocentricToNedConverter& converter)
{
	for (const auto& [entity, transform] : mEntityTransforms)
	{
		osg::Matrix matrix = transform->getMatrix();
		matrix.setTrans(converter.convertPosition(*getPosition(*entity)));
		transform->setMatrix(matrix);
	}
}

void VisEntityAttachables::setEntities(std::set<sim::Entity*> entities)
{
	// Remove old entities
	eraseIf(mEntityTransforms, [&](const auto& i) {
		bool shouldRemove = (entities.find(i.first) == entities.end());
		if (shouldRemove)
		{
			removeChild(i.second);
		}
		return shouldRemove;
	});

	// Add new entities
	for (const auto& entity : entities)
	{
		if (mEntityTransforms.find(entity) == mEntityTransforms.end())
		{
			osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform();
			transform->addChild(createNode(entity));
			mEntityTransforms[entity] = transform;
			addChild(transform);
		}
	}
}

} // namespace skybolt