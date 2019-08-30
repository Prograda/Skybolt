/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/VisObject.h>
#include <AircraftHud/HudDrawer.h>

namespace skybolt {

class VisHud : public vis::VisObject, public HudDrawer
{
public:
	VisHud(float aspectRatio);
	~VisHud();

	// VisObject interface
	virtual void setPosition(const osg::Vec3f &position) {}
	virtual void setOrientation(const osg::Quat &orientation) {}

	virtual osg::Vec3f getPosition() const {return osg::Vec3f();}
	virtual osg::Quat getOrientation() const {return osg::Quat();}

	virtual void updatePreRender(const vis::RenderContext& context);

	virtual osg::Node* _getNode() const;

	// HudDrawer interface
	virtual void drawLine(const glm::vec2 &p0, const glm::vec2 &p1);
	virtual void drawLineDashed(const glm::vec2 &p0, const glm::vec2 &p1, const DashedLineParams& params);
	virtual void drawText(const glm::vec2 &p, const std::string &message, float rotation, float size = -1);
	virtual void drawSolidBox(const glm::vec2 &position, float height, float width);
	
private:
	void nextFrame();

private:
	osg::Camera* mCamera;
	osg::ref_ptr<osg::Geode> mGeode;
	osg::Vec3Array* mLineVertices;
	osg::Vec3Array* mQuadVertices;
	std::unique_ptr<class TextPool> mTextPool;
};

} // namespace skybolt