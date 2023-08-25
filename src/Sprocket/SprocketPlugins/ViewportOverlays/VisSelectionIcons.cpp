/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisSelectionIcons.h"
#include <Sprocket/Scenario/ScenarioObject.h>
#include <Sprocket/Viewport/VisIcon.h>
#include <SkyboltCommon/ContainerUtility.h>
#include <SkyboltCommon/VectorUtility.h>
#include <SkyboltEngine/SimVisBinding/GeocentricToNedConverter.h>
#include <SkyboltVis/VisibilityCategory.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>
#include <osg/Geode>

namespace skybolt {

using namespace sim;

VisSelectionIcons::VisSelectionIcons(const vis::ShaderPrograms& programs, const osg::ref_ptr<osg::Texture>& iconTexture)
{
	setNodeMask(~vis::VisibilityCategory::shadowCaster);
	osg::ref_ptr<osg::StateSet> ss = createVisIconStateSet(programs.getRequiredProgram("hudGeometry"));
	ss->setTextureAttribute(0, iconTexture, osg::StateAttribute::ON);
	setStateSet(ss);
}

void VisSelectionIcons::syncVis(const GeocentricToNedConverter& converter)
{
	for (const auto& [object, transform] : mObjectTransforms)
	{
		if (auto position = object->getWorldPosition(); position)
		{
			osg::Matrix matrix = transform->getMatrix();
			matrix.setTrans(converter.convertPosition(*position));
			transform->setMatrix(matrix);
		}
	}
}

void VisSelectionIcons::setSelection(const std::vector<ScenarioObjectPtr>& objects)
{
	// Remove old entities
	eraseIf(mObjectTransforms, [&](const auto& i) {
		bool shouldRemove = !contains(objects, i.first);
		if (shouldRemove)
		{
			removeChild(i.second);
		}
		return shouldRemove;
	});

	// Add new entities
	for (const auto& object : objects)
	{
		if (object->getWorldPosition())
		{
			if (mObjectTransforms.find(object) == mObjectTransforms.end())
			{
				osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform();
				transform->addChild(createVisIconGeode(0.04f));
				mObjectTransforms[object] = transform;
				addChild(transform);
			}
		}
	}
}

} // namespace skybolt