/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <ExamplesCommon/HelpDisplayRenderOperation.h>
#include <ExamplesCommon/HelpDisplayToggleEventListener.h>
#include <ExamplesCommon/VisRootUtil.h>
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
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/System/System.h>

#include <SkyboltVis/Camera.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>
#include <SkyboltVis/RenderOperation/RenderOperationOrder.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Window/StandaloneWindow.h>

#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>

using namespace skybolt;
using namespace skybolt::sim;
using namespace skybolt::vis;

static void createEntities(const EntityFactory& entityFactory, World& world, CameraControllerSelector& cameraControllerSelector)
{
	world.addEntity(entityFactory.createEntity("Stars"));
	world.addEntity(entityFactory.createEntity("SunBillboard"));
	world.addEntity(entityFactory.createEntity("MoonBillboard"));

	EntityPtr planet = entityFactory.createEntity("PlanetEarth");
	world.addEntity(planet);

	cameraControllerSelector.setTargetId(planet->getId());
}

static osg::ref_ptr<HelpDisplayRenderOperation> createHelpDisplay()
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

	osg::ref_ptr<HelpDisplayRenderOperation> helpDisplay = new HelpDisplayRenderOperation();
	helpDisplay->setMessage(helpMessage);
	return helpDisplay;
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
		engineRoot->scenario->world.addEntity(simCamera);
		
		auto visRoot = createExampleVisRoot();

		// Attach camera to window
		vis::WindowPtr window = createExampleWindow();
		osg::ref_ptr<vis::RenderCameraViewport> viewport = createAndAddViewportToWindowWithEngine(*window, *engineRoot);
		viewport->setCamera(getVisCamera(*simCamera));
		visRoot->addWindow(window);

		// Create input
		auto inputPlatform = std::make_shared<InputPlatformOsg>(window->getView());
		CameraInputAxes axes = createDefaultCameraInputAxes(*inputPlatform);

		// Create systems
		engineRoot->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, toValuesVector(axes)));

		auto cameraInputSystem = std::make_shared<CameraInputSystem>(inputPlatform, axes);
		configure(*cameraInputSystem, window->getWidth(), engineRoot->engineSettings);
		connectToCamera(*cameraInputSystem, simCamera);
		engineRoot->systemRegistry->push_back(cameraInputSystem);

		osg::ref_ptr<HelpDisplayRenderOperation> helpDisplay = createHelpDisplay();
		window->getRenderOperationSequence().addOperation(helpDisplay, (int)RenderOperationOrder::Hud);
		auto helpDisplayToggleEventListener = std::make_shared<HelpDisplayToggleEventListener>(helpDisplay);
		inputPlatform->getEventEmitter()->addEventListener<KeyEvent>(helpDisplayToggleEventListener.get());

//#define SHOW_STATS
#ifdef SHOW_STATS
		engineRoot->systemRegistry->push_back(std::make_shared<StatsDisplaySystem>(&visRoot->getViewer(), window->getView(), viewport->getFinalRenderTarget()->getOsgCamera()));
#endif

		// Create entities
		createEntities(*engineRoot->entityFactory, engineRoot->scenario->world, *simCamera->getFirstComponentRequired<CameraControllerComponent>());

		// Run loop
		runMainLoop(*visRoot, *engineRoot, UpdateLoop::neverExit);
	}
	catch (const std::exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}
