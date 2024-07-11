/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisNameLabels.h"
#include "GeocentricToNedConverter.h"
#include "Components/TemplateNameComponent.h"
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltVis/OsgGeometryHelpers.h>
#include <SkyboltVis/OsgTextHelpers.h>
#include <SkyboltVis/VisibilityCategory.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <osg/Depth>
#include <osg/Geode>
#include <osgText/Text>

namespace skybolt {

using namespace sim;

VisNameLabels::VisNameLabels(World* world, const osg::ref_ptr<osg::Group>& parent, const vis::ShaderPrograms& programs) :
	SimVisObjectsReflector<osg::MatrixTransform*>(world, parent)
{
	mGroup->setNodeMask(~vis::VisibilityCategory::shadowCaster);
	mGroup->setStateSet(vis::createTransparentTextStateSet(programs.getRequiredProgram("hudText")));
}

VisNameLabels::~VisNameLabels() = default;

void VisNameLabels::syncVis(const GeocentricToNedConverter& converter)
{
	for (const auto& entry : getObjectsMap())
	{
		Entity* entity = entry.first;
		osg::MatrixTransform* transform = entry.second;

		if (applyVisibility(*entity, transform))
		{
			osg::Matrix matrix = transform->getMatrix();
			matrix.setTrans(converter.convertPosition(*getPosition(*entity)));
			transform->setMatrix(matrix);
		}
	}
}

std::optional<osg::MatrixTransform*> VisNameLabels::createObject(const sim::EntityPtr& entity)
{
	std::optional<sim::Vector3> position = getPosition(*entity);
	if (position && entity->getFirstComponent<TemplateNameComponent>())
	{
		std::string name = getName(*entity);
		if (!name.empty())
		{
			osgText::Text* text = new osgText::Text();
			text->setFont(vis::getDefaultFont());
			text->setText(name);
			text->setCharacterSize(0.12); // scaled in shader to be approximately font point size / 100
			vis::configureDrawable(*text);
			text->setCullingActive(false);

			osg::Geode* geode = new osg::Geode();
			geode->addDrawable(text);

			osg::MatrixTransform* transform = new osg::MatrixTransform();
			transform->addChild(geode);
			return transform;
		}
	}
	return std::nullopt;
}

} // namespace skybolt