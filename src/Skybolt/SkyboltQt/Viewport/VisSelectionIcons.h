/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltQt/SkyboltQtFwd.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/SimVisBinding/SimVisBinding.h>
#include <SkyboltVis/SkyboltVisFwd.h>
#include <osg/Group>
#include <osg/MatrixTransform>

namespace skybolt {

osg::ref_ptr<osg::Texture> createEntitySelectionIcon(int width = 64);

//! Displays an osg::Node, implemented by the derived class, at the position of each selected object
class VisSelectionIcons : public SimVisBinding
{
public:
	VisSelectionIcons(const osg::ref_ptr<osg::Group>& parent, const vis::ShaderPrograms& programs, const osg::ref_ptr<osg::Texture>& iconTexture = createEntitySelectionIcon());
	~VisSelectionIcons();

	void syncVis(const GeocentricToNedConverter& converter) override;

	void setSelection(const std::vector<ScenarioObjectPtr>& objects);

private:
	osg::ref_ptr<osg::Group> mGroup;
	std::map<ScenarioObjectPtr, osg::ref_ptr<osg::MatrixTransform>> mObjectTransforms;
};

} // namespace skybolt