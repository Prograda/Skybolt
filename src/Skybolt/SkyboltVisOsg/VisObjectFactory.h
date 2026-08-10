/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVisOsg/SkyboltVisOsgFwd.h"
#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltVis/VisFactory.h>

namespace skybolt {
namespace vis {

struct VisObjectFactoryConfig
{
	vis::ScenePtr scene;
	NonNullPtr<vis::VisFactoryRegistry> visFactoryRegistry;
	ComponentFactoryRegistryPtr componentFactoryRegistry;
	const vis::ShaderPrograms* programs;
};

class VisObjectFactory
{
public:
	VisObjectFactory(const VisObjectFactoryConfig& config);

private:
	vis::ScenePtr mScene;
	NonNullPtr<vis::VisFactoryRegistry> mVisFactoryRegistry;
	const ComponentFactoryRegistryPtr mComponentFactoryRegistry;
	NonNullPtr<const vis::ShaderPrograms> mPrograms;
	vis::ModelFactoryPtr mModelFactory;
	vis::TextureCachePtr mTextureCache;
};

} // namespace vis
} // namespace skybolt
