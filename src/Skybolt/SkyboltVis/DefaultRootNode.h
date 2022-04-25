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
	~DefaultRootNode() override;

	void setPosition(const osg::Vec3d &position) override;
	void setOrientation(const osg::Quat &orientation) override;
	void setTransform(const osg::Matrix& m) override;

	osg::Vec3d getPosition() const override { return mTransform->getMatrix().getTrans(); }
	osg::Quat getOrientation() const override { return mTransform->getMatrix().getRotate(); }
	osg::Matrix getTransform() const override { return mTransform->getMatrix(); }

	osg::Node* _getNode() const override { return mSwitch; }

	void setVisible(bool visible) override;
	bool isVisible() const override;

protected:
	osg::ref_ptr<osg::Switch> mSwitch;
	osg::ref_ptr<osg::MatrixTransform> mTransform;
};

} // namespace vis
} // namespace skybolt
