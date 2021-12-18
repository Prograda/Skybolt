/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltVis/VisObject.h>
#include <AircraftHud/HudDrawer.h>

namespace skybolt {

class VisHud : public HudDrawer, public osg::Group
{
public:
	VisHud();
	~VisHud();

	void setAspectRatio(double value);

	void clear();

	void setColor(const osg::Vec4f& color);

public:
	// HudDrawer interface
	void drawLine(const glm::vec2 &p0, const glm::vec2 &p1) override;
	void drawLineDashed(const glm::vec2 &p0, const glm::vec2 &p1, const DashedLineParams& params) override;
	void drawText(const glm::vec2 &p, const std::string &message, float rotation = 0.f, float size = -1, Alignment alignment = Alignment::Left) override;
	void drawSolidBox(const glm::vec2 &position, float width, float height) override;
	
private:
	void traverse(osg::NodeVisitor& visitor) override;
	void setDirty();

private:
	osg::ref_ptr<osg::Camera> mCamera;
	osg::ref_ptr<osg::Geode> mPrimitivesGeode;
	osg::ref_ptr<osg::Geode> mTextGeode;
	osg::ref_ptr<osg::Vec3Array> mLineVertices;
	osg::ref_ptr<osg::Vec3Array> mQuadVertices;
	osg::ref_ptr<osg::Uniform> mColorUniform;
	std::unique_ptr<class TextPool> mTextPool;
	bool mDirty = false;
};

} // namespace skybolt