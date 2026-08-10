/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/FactoryRegistries.h>
#include <SkyboltVisOsg/Rect.h>
#include <SkyboltVisOsg/VisRoot.h>
#include <osgViewer/CompositeViewer>

namespace skybolt {

std::unique_ptr<vis::VisRoot> createVisRoot(const EngineRoot& engineRoot)
{
	auto visRoot = std::make_unique<vis::VisRoot>(vis::VisRootConfig{
		.assetPackagePaths = engineRoot.getAssetPackagePaths(),
		.visFactoryRegistry = valueOrThrowException(getExpectedRegistry<vis::VisFactoryRegistry>(*engineRoot.factoryRegistries)),
		.componentFactoryRegistry = engineRoot.componentFactoryRegistry,
		.world = &engineRoot.scenario->world,
		.systemRegistry = engineRoot.systemRegistry,
		});
	return visRoot;
}

void enableQuitOnEscapePress(const vis::VisRoot& visRoot)
{
	visRoot.getViewer().setKeyEventSetsDone(osgGA::GUIEventAdapter::KEY_Escape);
}

} // namespace skybolt
