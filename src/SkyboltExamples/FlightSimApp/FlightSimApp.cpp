/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <ExamplesCommon/EntityInputSystem.h>
#include <ExamplesCommon/HudSystem.h>
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
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/CameraController/OrbitCameraController.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
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
	CameraViewSelector(const CameraControllerSelectorPtr& cameraController, const std::shared_ptr<HudSystem>& hudSystem, const std::function<void(bool)>& hudVisibilitySwitch) :
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
	CameraControllerSelectorPtr mCameraController;
	std::shared_ptr<HudSystem> mHudSystem;
	std::function<void(bool)> mHudVisibilitySwitch;
};

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

		// Configure external view camera controller
		auto cameraControllerComponent = simCamera->getFirstComponentRequired<CameraControllerComponent>();
		auto cameraController = std::static_pointer_cast<CameraControllerSelector>(cameraControllerComponent->cameraController);
		auto orbitController = cameraController->getControllerOfType<OrbitCameraController>();
		orbitController->setLagTimeConstant(0.3);
		orbitController->setTargetOffset(sim::Vector3(0, 0, -3));
		orbitController->setZoom(0.8);

		// Attach camera to window
		std::unique_ptr<vis::StandaloneWindow> window = createExampleWindow();
		osg::ref_ptr<vis::RenderCameraViewport> viewport = createAndAddViewportToWindowWithEngine(*window, *engineRoot);
		viewport->setCamera(getVisCamera(*simCamera));

		// Create input
		auto inputPlatform = std::make_shared<InputPlatformOsg>(window->getViewerPtr());
		std::vector<LogicalAxisPtr> axes = createHelicopterInputAxesKeyboard(*inputPlatform);

		// Create systems
		engineRoot->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, window.get(), axes));
		engineRoot->systemRegistry->push_back(std::make_shared<CameraInputSystem>(simCamera, inputPlatform, std::vector<LogicalAxisPtr>()));
		
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

		osg::ref_ptr<osg::Camera> overlayCamera = viewport->getFinalRenderTarget()->getOsgCamera();
		auto helpDisplaySystem = std::make_shared<HelpDisplaySystem>(overlayCamera);
		helpDisplaySystem->setMessage(helpMessage);
		engineRoot->systemRegistry->push_back(helpDisplaySystem);

		auto entityInputSystem = std::make_shared<EntityInputSystem>(axes);
		engineRoot->systemRegistry->push_back(entityInputSystem);

		auto hudSystem = std::make_shared<HudSystem>(overlayCamera, [&] { return getVisCamera(*simCamera)->getFovY(); });
		engineRoot->systemRegistry->push_back(hudSystem);

		// Register input event listeners
		auto hudVisibilitySwitch = [&](bool visible) {
			hudSystem->setEnabled(visible);
		};

		auto cameraViewSelector = std::make_shared<CameraViewSelector>(cameraController, hudSystem, hudVisibilitySwitch);
		inputPlatform->getEventEmitter()->addEventListener<KeyEvent>(cameraViewSelector.get());

		auto helpDisplayToggleEventListener = std::make_shared<HelpDisplayToggleEventListener>(helpDisplaySystem);
		inputPlatform->getEventEmitter()->addEventListener<KeyEvent>(helpDisplayToggleEventListener.get());

//#define SHOW_STATS
#ifdef SHOW_STATS
		engineRoot->systemRegistry->push_back(std::make_shared<StatsDisplaySystem>(&window->getViewer(), overlayCamera));
#endif

		// Create entities
		createEnvironmentEntities(*engineRoot->entityFactory, *engineRoot->simWorld);
		
		EntityPtr aircraft = engineRoot->entityFactory->createEntity("UH60");
		engineRoot->simWorld->addEntity(aircraft);
		
		sim::LatLonAlt boeingFieldPosition(47.537 * math::degToRadD(), -122.307 * math::degToRadD(), 500);
		setPosition(*aircraft, toGeocentric(sim::LatLonAltPosition(boeingFieldPosition)).position);
		setOrientation(*aircraft, toGeocentric(sim::LtpNedOrientation(math::quatIdentity()), sim::toLatLon(boeingFieldPosition)).orientation);

		// Configure systems for player's aircraft
		entityInputSystem->setEntity(aircraft);
		hudSystem->setEntity(aircraft.get());
		cameraController->setTarget(aircraft.get());

		// Set time of day
		engineRoot->scenario.startJulianDate = sim::calcJulianDate(/* year */ 2030, /* month */ 4, /* day */ 6, /* hour */ 20);

		// Run loop
		runMainLoop(*window, *engineRoot, UpdateLoop::neverExit);
	}
	catch (const std::exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}
