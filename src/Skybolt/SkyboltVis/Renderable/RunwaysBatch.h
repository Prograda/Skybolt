/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "DefaultRootNode.h"
#include "OsgBox2.h"

namespace skybolt {
namespace vis {

struct Runway
{
	osg::Vec3f startPoint;
	osg::Vec3f endPoint;
	std::string startMarking;
	std::string endMarking;
	float width; //!< total width of all lanes
};

typedef std::vector<Runway> Runways;

class RunwaysBatch : public DefaultRootNode
{
public:
	struct Uniforms
	{
		osg::Uniform* modelMatrix;
	};

	RunwaysBatch(const Runways& runways, const osg::ref_ptr<osg::Program>& surfaceProgram, const osg::ref_ptr<osg::Program>& textProgram);

protected:
	void updatePreRender(const CameraRenderContext& context) override;

private:
	Uniforms mUniforms;
};

} // namespace vis
} // namespace skybolt
