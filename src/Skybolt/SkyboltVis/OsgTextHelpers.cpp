/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgTextHelpers.h"

#include <osg/BlendEquation>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Program>
#include <osg/StateSet>

namespace skybolt {
namespace vis {

osg::ref_ptr<osg::StateSet> createTransparentTextStateSet(const osg::ref_ptr<osg::Program>& program)
{
	osg::ref_ptr<osg::StateSet> ss = new osg::StateSet();
	ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	ss->setMode(GL_BLEND, osg::StateAttribute::ON);
	ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	depth->setFunction(osg::Depth::ALWAYS);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
	return ss;
}

osg::ref_ptr<osgText::Font> getDefaultFont()
{
	static osg::ref_ptr<osgText::Font> font = osgText::readRefFontFile("fonts/verdana.ttf"); // static so we only load the font once
	return font;
}

} // namespace vis
} // namespace skybolt
