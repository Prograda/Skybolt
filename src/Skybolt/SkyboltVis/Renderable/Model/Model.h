/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include <osg/Program>

namespace skybolt {
namespace vis {

struct ModelConfig
{
	osg::ref_ptr<osg::Node> node;
};

class Model : public DefaultRootNode
{
public:
	Model(const ModelConfig &params);
	~Model();

	void setMaxRenderDistance(float distance);

	void setVisibilityCategoryMask(uint32_t mask) override;

private:
	void updatePreRender(const RenderContext& context) override;

private:
	osg::Node* mNode;
	osg::Uniform* mModelMatrix;
};

} // namespace vis
} // namespace skybolt

