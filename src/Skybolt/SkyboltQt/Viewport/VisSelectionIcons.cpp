/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisSelectionIcons.h"

#include <SkyboltQt/Scenario/ScenarioObject.h>
#include <SkyboltQt/Viewport/VisIcon.h>
#include <SkyboltCommon/ContainerUtility.h>
#include <SkyboltCommon/VectorUtility.h>
#include <SkyboltEngine/SimVisBinding/GeocentricToNedConverter.h>
#include <SkyboltVis/VisibilityCategory.h>
#include <SkyboltVis/Shader/ShaderProgramRegistry.h>

#include <osg/Geode>
#include <osg/Texture2D>

namespace skybolt {

using namespace sim;

osg::ref_ptr<osg::Texture> createEntitySelectionIcon(int width)
{
	int height = width;
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	std::uint8_t* p = image->data();
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
#ifdef ENTITY_SELECTION_ICON_BOX
			bool edge = (x == 0 || y == 0 || x == (width - 1) || y == (height - 1));
			int alpha = edge ? 255 : 0;
#else // circle
			int rx = std::abs(x - width/2);
			int ry = std::abs(y - width/2);
			float r = glm::length(glm::vec2(float(rx), float(ry)));
			
			float rampUpStart = width/2 - 3;
			float rampUpEnd = rampUpStart+1;
			float rampDownStart = rampUpStart+2;
			float rampDownEnd = rampUpStart+3;
			int alpha = int(255.f * std::max(0.f, (glm::smoothstep(rampUpStart, rampUpEnd, r)) - std::max(0.f, glm::smoothstep(rampDownStart, rampDownEnd, r))));
#endif
			*p++ = 255;
			*p++ = 255;
			*p++ = 255;
			*p++ = alpha;
		}
	}

	return new osg::Texture2D(image);
}

VisSelectionIcons::VisSelectionIcons(const osg::ref_ptr<osg::Group>& parent, const vis::ShaderPrograms& programs, const osg::ref_ptr<osg::Texture>& iconTexture) :
	mGroup(new osg::Group)
{
	parent->addChild(mGroup);

	mGroup->setNodeMask(~vis::VisibilityCategory::shadowCaster);
	osg::ref_ptr<osg::StateSet> ss = createVisIconStateSet(programs.getRequiredProgram("hudGeometry"));
	ss->setTextureAttribute(0, iconTexture, osg::StateAttribute::ON);
	mGroup->setStateSet(ss);
}

VisSelectionIcons::~VisSelectionIcons()
{
	mGroup->getParent(0)->removeChild(mGroup);
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
			mGroup->removeChild(i.second);
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
				mGroup->addChild(transform);
			}
		}
	}
}

} // namespace skybolt