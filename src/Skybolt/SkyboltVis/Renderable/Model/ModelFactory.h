/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltVis/DefaultRootNode.h"
#include <osg/Material>
#include <osg/Program>
#include <functional>

namespace skybolt {
namespace vis {

typedef std::function<void(osg::StateSet& stateSet, const osg::Material& material)> StateSetModifier;
typedef std::map<std::string, StateSetModifier> NamedStateSetModifiers;

struct ModelFactoryConfig
{
	NamedStateSetModifiers stateSetModifiers;
	osg::ref_ptr<osg::Program> defaultProgram;
};

class ModelFactory : public DefaultRootNode
{
public:
	ModelFactory(const ModelFactoryConfig &config);
	
	enum class TextureRole
{
	Albedo,
	Normal,
	OcclusionRoughnessMetalness
};

	osg::ref_ptr<osg::Node> createModel(const std::string& filename, const std::vector<TextureRole>& textureRoles);

private:
	NamedStateSetModifiers mStateSetModifiers;
	osg::ref_ptr<osg::Program> mDefaultProgram;
};

} // namespace vis
} // namespace skybolt
