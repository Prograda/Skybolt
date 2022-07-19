/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <ExamplesCommon/HelpDisplaySystem.h>
#include <ExamplesCommon/HelpDisplayToggleEventListener.h>
#include <ExamplesCommon/WindowUtil.h>

#include <SkyboltEngine/CameraInputSystem.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/WindowUtil.h>
#include <SkyboltEngine/Diagnostics/StatsDisplaySystem.h>
#include <SkyboltEngine/Input/InputPlatformOsg.h>
#include <SkyboltEngine/Input/InputSystem.h>
#include <SkyboltEngine/Input/LogicalAxis.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltEngine/UpdateLoop/UpdateLoopUtility.h>

#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/System/System.h>

#include <SkyboltVis/Camera.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Window/StandaloneWindow.h>

#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>

using namespace skybolt;
using namespace skybolt::sim;
using namespace skybolt::vis;

static void createEntities(const EntityFactory& entityFactory, World& world, CameraController& cameraController)
{
	world.addEntity(entityFactory.createEntity("Stars"));
	world.addEntity(entityFactory.createEntity("SunBillboard"));
	world.addEntity(entityFactory.createEntity("MoonBillboard"));

	EntityPtr planet = entityFactory.createEntity("PlanetEarth");
	world.addEntity(planet);

	cameraController.setTarget(planet.get());
}

static std::shared_ptr<HelpDisplaySystem> createHelpDisplaySystem(const osg::ref_ptr<osg::Camera>& camera)
{
	std::string helpMessage =
R"(= Controls =
W: Zoom in
S: Zoom out
H: Toggle help message
Shift (hold): Camera tilt modifier
Esc: Exit
Mouse: Rotate camera
)";

	auto helpDisplaySystem = std::make_shared<HelpDisplaySystem>(camera);
	helpDisplaySystem->setMessage(helpMessage);
	return helpDisplaySystem;
}

int main(int argc, char *argv[])
{
	try
	{
		// Create engine
		auto params = EngineCommandLineParser::parse(argc, argv);
		std::unique_ptr<EngineRoot> engineRoot = EngineRootFactory::create(params);

		// Create camera
		EntityPtr simCamera = engineRoot->entityFactory->createEntity("Camera");
		engineRoot->simWorld->addEntity(simCamera);

		// Attach camera to window
		std::unique_ptr<vis::StandaloneWindow> window = createExampleWindow();
		osg::ref_ptr<vis::RenderCameraViewport> viewport = createAndAddViewportToWindowWithEngine(*window, *engineRoot);
		viewport->setCamera(getVisCamera(*simCamera));

		// Create input
		auto inputPlatform = std::make_shared<InputPlatformOsg>(window->getViewerPtr());
		std::vector<LogicalAxisPtr> axes = CameraInputSystem::createDefaultAxes(*inputPlatform);

		// Create systems
		engineRoot->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, window.get(), axes));
		engineRoot->systemRegistry->push_back(std::make_shared<CameraInputSystem>(simCamera, inputPlatform, axes));

		auto helpDisplaySystem = createHelpDisplaySystem(viewport->getFinalRenderTarget()->getOsgCamera());
		engineRoot->systemRegistry->push_back(helpDisplaySystem);
		auto helpDisplayToggleEventListener = std::make_shared<HelpDisplayToggleEventListener>(helpDisplaySystem);
		inputPlatform->getEventEmitter()->addEventListener<KeyEvent>(helpDisplayToggleEventListener.get());

//#define SHOW_STATS
#ifdef SHOW_STATS
		engineRoot->systemRegistry->push_back(std::make_shared<StatsDisplaySystem>(*window));
#endif

		// Create entities
		createEntities(*engineRoot->entityFactory, *engineRoot->simWorld, *simCamera->getFirstComponentRequired<CameraControllerComponent>()->cameraController);

		// Run loop
		runMainLoop(*window, *engineRoot, UpdateLoop::neverExit);
	}
	catch (const std::exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}
