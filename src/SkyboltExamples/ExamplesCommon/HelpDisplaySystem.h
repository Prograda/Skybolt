/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltSim/System/System.h"
#include "SkyboltVis/SkyboltVisFwd.h"

#include <osg/Camera>
#include <osg/ref_ptr>
#include <osg/Stats>

namespace skybolt {

class HelpDisplaySystem : public sim::System
{
public:
	HelpDisplaySystem(const osg::ref_ptr<osg::Camera>& camera);
	~HelpDisplaySystem();

	void setMessage(const std::string& message);

	void setVisible(bool visible);
	bool isVisible();

	void updatePostDynamics(const System::StepArgs& args) override;

private:
	osg::ref_ptr<class VisHud> mHud;
	osg::ref_ptr<osg::Camera> mCamera;
};

} // namespace skybolt