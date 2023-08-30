/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <ExamplesCommon/EntityInputSystem.h>
#include <ExamplesCommon/HudSystem.h>
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
#include <SkyboltSim/CameraController/OrbitCameraController.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltSim/System/System.h>

#include <SkyboltVis/Camera.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>
#include <SkyboltVis/RenderOperation/RenderOperationOrder.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Window/StandaloneWindow.h>

#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>

using namespace skybolt;
using namespace skybolt::sim;
using namespace skybolt::vis;

static void createEnvironmentEntities(const EntityFactory& entityFactory, World& world)
{
	world.addEntity(entityFactory.createEntity("Stars"));
	world.addEntity(entityFactory.createEntity("SunBillboard"));
	world.addEntity(entityFactory.createEntity("MoonBillboard"));
	world.addEntity(entityFactory.createEntity("PlanetEarth"));
}

static std::vector<LogicalAxisPtr> createHelicopterInputAxesKeyboard(const InputPlatform& inputPlatform)
{
	InputDevicePtr keyboard = inputPlatform.getInputDevicesOfType(InputDeviceTypeKeyboard)[0];
	float rate = 10;

	std::vector<LogicalAxisPtr> logicalAxes;
	logicalAxes.push_back(std::make_shared<KeyAxis>(keyboard, KC_DOWN, KC_UP, rate, rate, -1.0f, 1.0f));
	logicalAxes.push_back(std::make_shared<KeyAxis>(keyboard, KC_LEFT, KC_RIGHT, rate, rate, -1.0f, 1.0f));
	logicalAxes.push_back(std::make_shared<KeyAxis>(keyboard, KC_S, KC_W, 1.0, 0.0, 0.0f, 1.0f, 0.5f));
	logicalAxes.push_back(std::make_shared<KeyAxis>(keyboard, KC_A, KC_D, rate, rate, -1.0f, 1.0f));

	return logicalAxes;
}

class CameraViewSelector : public EventListener
{
public:
	CameraViewSelector(const CameraControllerComponentPtr& cameraController, const std::shared_ptr<HudSystem>& hudSystem, const std::function<void(bool)>& hudVisibilitySwitch) :
		mCameraController(cameraController),
		mHudSystem(hudSystem),
		mHudVisibilitySwitch(hudVisibilitySwitch)
	{
		assert(mCameraController);
		assert(mHudSystem);
		mCameraController->selectController("Cockpit");
	}

	void onEvent(const Event& event) override
	{
		if (const KeyEvent* keyEvent = dynamic_cast<const KeyEvent*>(&event))
		{
			if (keyEvent->type == KeyEvent::Pressed)
			{
				switch (keyEvent->code)
				{
					case KC_F1:
					{
						mCameraController->selectController("Cockpit");
						mHudSystem->setPitchLadderVisible(true);
						mHudVisibilitySwitch(true);
						break;
					}
					case KC_F2:
					{
						mCameraController->selectController("Follow");
						mHudSystem->setPitchLadderVisible(false);
						mHudVisibilitySwitch(true);
						break;
					}
				}
			}
		}
	}

private:
	CameraControllerComponentPtr mCameraController;
	std::shared_ptr<HudSystem> mHudSystem;
	std::function<void(bool)> mHudVisibilitySwitch;
};

static osg::ref_ptr<HelpDisplayRenderOperation> createHelpDisplay()
{
	std::string helpMessage =
R"(= Controls =
W: Collective up
S: Collective down
A: Pedel Left
D: Pedel Right
Up arrow: Cyclic forward
Down arrow: Cyclic aft
Left arrow: Cyclic left
Right arrow: Cyclic right
F1: Cockpit view
F2: External view
H: Toggle help message
Esc: Exit
Mouse: Pan camera
)";

	osg::ref_ptr<HelpDisplayRenderOperation> helpDisplay = new HelpDisplayRenderOperation();
	helpDisplay->setMessage(helpMessage);
	return helpDisplay;
}

int main(int argc, char *argv[])
{
	try
	{
		// Parse commandline arguments
		boost::program_options::options_description desc;
		desc.add_options()("multiwindow", "demonstrate rendering to multiple display windows");
		EngineCommandLineParser::addOptions(desc);
		auto params = EngineCommandLineParser::parse(argc, argv, desc);

		// Create engine
		std::unique_ptr<EngineRoot> engineRoot = EngineRootFactory::create(params);

		// Create camera
		EntityPtr simCamera = engineRoot->entityFactory->createEntity("Camera");
		engineRoot->scenario->world.addEntity(simCamera);

		// Configure external view camera controller
		auto cameraControllerComponent = simCamera->getFirstComponentRequired<CameraControllerComponent>();
		auto orbitController = cameraControllerComponent->getControllerOfType<OrbitCameraController>();
		orbitController->setLagTimeConstant(0.3);
		orbitController->setTargetOffset(sim::Vector3(0, 0, -3));
		orbitController->setZoom(0.8);

		auto visRoot = createExampleVisRoot();

		// Attach camera to window
		vis::WindowPtr window = createExampleWindow();

		osg::ref_ptr<vis::RenderCameraViewport> viewport = createAndAddViewportToWindowWithEngine(*window, *engineRoot);
		viewport->setCamera(getVisCamera(*simCamera));
		visRoot->addWindow(window);

		CameraControllerComponentPtr cameraControllerComponent2;
		if (params.count("multiwindow"))
		{
			sim::EntityPtr simCamera2 = engineRoot->entityFactory->createEntity("Camera");
			engineRoot->scenario->world.addEntity(simCamera2);
			
			cameraControllerComponent2 = simCamera2->getFirstComponentRequired<CameraControllerComponent>();
			cameraControllerComponent2->selectController("Follow");

			vis::WindowPtr window2 = std::make_unique<vis::StandaloneWindow>(vis::RectI(1080, 0, 1080, 720));
			osg::ref_ptr<vis::RenderCameraViewport> viewport2 = createAndAddViewportToWindowWithEngine(*window2, *engineRoot);
			viewport2->setCamera(getVisCamera(*simCamera2));
			visRoot->addWindow(window2);
		}

		// Create input
		auto inputPlatform = std::make_shared<InputPlatformOsg>(window->getView());
		std::vector<LogicalAxisPtr> axes = createHelicopterInputAxesKeyboard(*inputPlatform);

		// Create systems
		engineRoot->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, axes));

		auto cameraInputSystem = std::make_shared<CameraInputSystem>(inputPlatform);
		configure(*cameraInputSystem, window->getWidth(), engineRoot->engineSettings);
		connectToCamera(*cameraInputSystem, simCamera);
		engineRoot->systemRegistry->push_back(cameraInputSystem);

		osg::ref_ptr<osg::Camera> overlayCamera = viewport->getFinalRenderTarget()->getOsgCamera();

		osg::ref_ptr<HelpDisplayRenderOperation> helpDisplay = createHelpDisplay();
		window->getRenderOperationSequence().addOperation(helpDisplay, (int)RenderOperationOrder::Hud);

		auto entityInputSystem = std::make_shared<EntityInputSystem>(axes);
		engineRoot->systemRegistry->push_back(entityInputSystem);

		auto hudSystem = std::make_shared<HudSystem>(overlayCamera, [&] { return getVisCamera(*simCamera)->getFovY(); });
		engineRoot->systemRegistry->push_back(hudSystem);

		// Register input event listeners
		auto hudVisibilitySwitch = [&](bool visible) {
			hudSystem->setEnabled(visible);
		};

		auto cameraViewSelector = std::make_shared<CameraViewSelector>(cameraControllerComponent, hudSystem, hudVisibilitySwitch);
		inputPlatform->getEventEmitter()->addEventListener<KeyEvent>(cameraViewSelector.get());

		auto helpDisplayToggleEventListener = std::make_shared<HelpDisplayToggleEventListener>(helpDisplay);
		inputPlatform->getEventEmitter()->addEventListener<KeyEvent>(helpDisplayToggleEventListener.get());

//#define SHOW_STATS
#ifdef SHOW_STATS
		engineRoot->systemRegistry->push_back(std::make_shared<StatsDisplaySystem>(&visRoot->getViewer(), window->getView(), overlayCamera));
#endif

		// Create entities
		createEnvironmentEntities(*engineRoot->entityFactory, engineRoot->scenario->world);
		
		EntityPtr aircraft = engineRoot->entityFactory->createEntity("UH60");
		engineRoot->scenario->world.addEntity(aircraft);

		sim::LatLonAlt boeingFieldPosition(47.537 * math::degToRadD(), -122.307 * math::degToRadD(), 500);
		setPosition(*aircraft, toGeocentric(sim::LatLonAltPosition(boeingFieldPosition)).position);
		setOrientation(*aircraft, toGeocentric(sim::LtpNedOrientation(math::quatIdentity()), sim::toLatLon(boeingFieldPosition)).orientation);

		// Configure systems for player's aircraft
		entityInputSystem->setEntity(aircraft);
		hudSystem->setEntity(aircraft.get());
		cameraControllerComponent->setTargetId(aircraft->getId());

		if (cameraControllerComponent2)
		{
			cameraControllerComponent2->setTargetId(aircraft->getId());
		}

		// Set time of day
		engineRoot->scenario->startJulianDate = sim::calcJulianDate(/* year */ 2030, /* month */ 4, /* day */ 6, /* hour */ 20);

		// Run loop
		runMainLoop(*visRoot, *engineRoot, UpdateLoop::neverExit);
	}
	catch (const std::exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}
