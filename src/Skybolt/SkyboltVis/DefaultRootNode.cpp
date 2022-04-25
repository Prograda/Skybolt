/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "SkyboltVis/DefaultRootNode.h"

using namespace skybolt::vis;

DefaultRootNode::DefaultRootNode() :
	mSwitch(new osg::Switch),
	mTransform(new osg::MatrixTransform)
{
	mSwitch->addChild(mTransform);
}

DefaultRootNode::~DefaultRootNode()
{
}

void DefaultRootNode::setPosition(const osg::Vec3d &position)
{
	osg::Matrix mat = mTransform->getMatrix();
	mat.setTrans(position);
	mTransform->setMatrix(mat);
}

void DefaultRootNode::setOrientation(const osg::Quat &orientation)
{
	osg::Matrix mat = mTransform->getMatrix();
	mat.setRotate(orientation);
	mTransform->setMatrix(mat);
}

void DefaultRootNode::setTransform(const osg::Matrix& m)
{
	mTransform->setMatrix(m);
}

void DefaultRootNode::setVisible(bool visible)
{
	visible ? mSwitch->setAllChildrenOn() : mSwitch->setAllChildrenOff();
}

bool DefaultRootNode::isVisible() const
{
	return mSwitch->getChildValue(mTransform);
}
