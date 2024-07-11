/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/RenderOperation/RenderOperation.h>

#include <osg/Camera>
#include <osg/ref_ptr>
#include <osg/Stats>

namespace skybolt {

class HelpDisplayRenderOperation : public vis::RenderOperation
{
public:
	HelpDisplayRenderOperation();
	~HelpDisplayRenderOperation() override = default;

	void setMessage(const std::string& message);

	void setVisible(bool visible);
	bool isVisible();

	void updatePreRender(const vis::RenderContext& context);

private:
	osg::ref_ptr<class VisHud> mHud;
};

} // namespace skybolt