/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/NodeVisitor>

namespace skybolt {
namespace vis {

struct ModelPreparerConfig
{
	bool generateTangents = false;
};

class ModelPreparer : public osg::NodeVisitor
{
public:
	ModelPreparer(const ModelPreparerConfig& config);
	virtual void apply(osg::Node &node);

private:
    virtual void apply(osg::Geode &geode); 

private:
	bool mGenerateTangents;
};

} // namespace vis
} // namespace skybolt
