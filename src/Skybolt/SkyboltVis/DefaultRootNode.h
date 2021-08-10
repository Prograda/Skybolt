/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "RootNode.h"
#include <osg/MatrixTransform>
#include <osg/Switch>

namespace skybolt {
namespace vis {

class DefaultRootNode : public RootNode
{
public:
	DefaultRootNode();
	virtual ~DefaultRootNode();

	// RootNode Implementation
	virtual void setPosition(const osg::Vec3d &position);
	void setOrientation(const osg::Quat &orientation);

	osg::Vec3d getPosition() const { return mTransform->getMatrix().getTrans(); }
	osg::Quat getOrientation() const { return mTransform->getMatrix().getRotate(); }

	osg::Node* _getNode() const { return mSwitch; }

	void setVisible(bool visible) override;
	bool isVisible() const override;

protected:
	osg::ref_ptr<osg::Switch> mSwitch;
	osg::ref_ptr<osg::MatrixTransform> mTransform;
};

} // namespace vis
} // namespace skybolt
