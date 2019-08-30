/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/NodeVisitor>

namespace skybolt {
namespace vis {

class ModelPreparer : public osg::NodeVisitor
{
public:
	ModelPreparer();
	virtual void apply(osg::Node &node);

private:
    virtual void apply(osg::Geode &geode); 
};

} // namespace vis
} // namespace skybolt
