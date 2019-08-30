/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "UpdateLoop.h"
#include "VisHud.h"

#include <SkyboltSim/CameraController/OrbitCameraController.h>
#include <SkyboltSim/CameraController/FreeCameraController.h>
#include <SkyboltSim/CameraController/PlanetCameraController.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltSim/System/SimStepper.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltSim/World.h>

#include <SkyboltVis/Camera.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/RenderTarget/RenderTargetSceneAdapter.h>
#include <SkyboltVis/RenderTarget/RenderTexture.h>
#include <SkyboltVis/RenderTarget/Viewport.h>
#include <SkyboltVis/RenderTarget/ViewportHelpers.h>
#include <SkyboltVis/Window/StandaloneWindow.h>

#include <SkyboltEngine/GetExecutablePath.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/Input/InputPlatformOis.h>
#include <SkyboltEngine/Input/LogicalAxis.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/GeocentricToNedConverter.h>

#include <SkyboltCommon/Exception.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/OptionalUtility.h>
#include <SkyboltCommon/Random.h>

using namespace skybolt;
using namespace skybolt::sim;
using namespace skybolt::vis;

class StatsDisplaySystem : public System
{
public:
	StatsDisplaySystem(const Window& window, Scene& Scene)
	{
		osgViewer::Viewer& viewer = *window._getViewer();

		mViewerStats = viewer.getStats();
		mCameraStats = viewer.getCamera()->getStats();

		mViewerStats->collectStats("frame_rate", true);
		mCameraStats->collectStats("gpu", true);
		
		if (false)
		{
			mViewerStats->collectStats("event", true);
			mViewerStats->collectStats("update", true);
			mCameraStats->collectStats("rendering", true);
			mCameraStats->collectStats("scene", true);
		}

		const float aspectRatio = (float)window.getWidth() / (float)window.getHeight();
		mStatsHud = std::make_unique<VisHud>(aspectRatio);
		Scene.addObject(mStatsHud.get());
	}

	void updatePostDynamics(const System::StepArgs& args) override
	{
		int line = 0;
		int frameNumber = mViewerStats->getLatestFrameNumber() - 1;

		auto attributes = mViewerStats->getAttributeMap(frameNumber);
		auto attributes2 = mCameraStats->getAttributeMap(frameNumber);
		attributes.insert(attributes2.begin(), attributes2.end());

		for (const auto& value : attributes)
		{
			mStatsHud->drawText(glm::vec2(-0.9f, 0.9f - line * 0.05f), value.first + ": " + std::to_string(value.second), 0.0f, 0.2);
			++line;
		}
	}

private:
	osg::Stats* mViewerStats;
	osg::Stats* mCameraStats;
	std::unique_ptr<VisHud> mStatsHud;
};

#include <fstream>
#include <istream>
#include <ostream>

class DebugPositionLoaderSystem : public EventListener, public System
{
public:
	DebugPositionLoaderSystem(InputPlatform* inputPlatform, Node* node) :
		mInputPlatform(inputPlatform),
		mNode(node)
	{
		mInputPlatform->addEventListener<KeyEvent>(this);
		assert(mNode);
	}

	~DebugPositionLoaderSystem()
	{
		mInputPlatform->removeEventListener(this);
	}

private:
	void onEvent(const Event& event) override
	{
		if (const auto& keyEvent = dynamic_cast<const KeyEvent*>(&event))
		{
			if (keyEvent->code == KC_P)
			{
				sim::Vector3 pos = mNode->getPosition();

				std::ofstream file("camera.txt");
				file << pos.x << " " << pos.y << " " << pos.z << std::endl;
				file.close();

				return;
			}
			else if (keyEvent->code == KC_O)
			{
				sim::Vector3 pos;
				double v;

				std::ifstream file("camera.txt");
				file >> v;
				pos.x = v;
				file >> v;
				pos.y = v;
				file >> v;
				pos.z = v;
				file.close();

				mNode->setPosition(pos);

				return;
			}
		}
	}

private:
	InputPlatform* mInputPlatform;
	Node* mNode;
};

class Ships : public System
{
public:
	Ships(const EntityFactory& factory, World& simWorld, const LatLon& startPosition, float heading) :
		mHeading(heading)
	{
		skybolt::Random random(0);
		EntityPtr entity;

		for (int i = 0; i < 300; ++i)
		{
			LatLon entityPosition = startPosition;

			if (i > 0)
			{
				entityPosition.lat += 0.003 * (random.unitRand() - 0.5);
				entityPosition.lon += 0.003 * (random.unitRand() - 0.5);
			}

			auto geocentricPosition = sim::toGeocentric(sim::LatLonAltPosition(sim::toLatLonAlt(entityPosition, 0)));
			auto geocentricOrientation = sim::toGeocentric(sim::LtpNedOrientation(glm::angleAxis((double)heading, Vector3(0, 0, 1))), entityPosition);
			entity = factory.createEntity("Frigate", "", geocentricPosition.position, geocentricOrientation.orientation);
			entity->setDynamicsEnabled(false);
			simWorld.addEntity(entity);

			mShips.push_back(entity);
			mShipPositions.push_back(sim::LatLonAltPosition(sim::toLatLonAlt(entityPosition, 0)));
		}
	}

	void updatePostDynamics(const System::StepArgs& args) override
	{
		const float shipSpeed = 8.f;
		int i = 0;
		for (const EntityPtr& ship : mShips)
		{
			auto& position = mShipPositions[i];
			float displacement = shipSpeed * args.dtSim;
			glm::vec2 displacementVec = skybolt::math::vec2Rotate(glm::vec2(displacement, 0), mHeading);
			sim::LatLon displacementLatLon = cartesianNeToLatLon(toLatLon(position.position), glm::dvec2(displacementVec.x, displacementVec.y));
			position.position.lat = displacementLatLon.lat;
			position.position.lon = displacementLatLon.lon;

			auto geocentricPosition = sim::toGeocentric(sim::LatLonAltPosition(position));
			setPosition(*ship, geocentricPosition.position);
			++i;
		}
	}

private:
	const float mHeading;
	std::vector<EntityPtr> mShips;
	std::vector<sim::LatLonAltPosition> mShipPositions;
};

typedef std::shared_ptr<InputPlatformOis> InputPlatformOisPtr;

class InputSystem : public System
{
public:
	InputSystem(const InputPlatformOisPtr& inputPlatform, Window* window, const std::vector<LogicalAxisPtr>& axes) :
		mInputPlatform(inputPlatform),
		mWindow(window),
		mAxes(axes)
	{
		assert(mInputPlatform);
		assert(mWindow);
	}

	void updatePostDynamics(const System::StepArgs& args)
	{
		mInputPlatform->update();
		mInputPlatform->setWindowWidth(mWindow->getWidth());
		mInputPlatform->setWindowHeight(mWindow->getHeight());

		for (const LogicalAxisPtr& axis : mAxes)
		{
			axis->update(args.dtWallClock);
		}
	}

private:
	InputPlatformOisPtr mInputPlatform;
	Window* mWindow;
	std::vector<LogicalAxisPtr> mAxes;
};

vis::CameraPtr getCamera(const Entity& camera)
{
	return static_cast<const CameraSimVisBinding&>(*camera.getFirstComponent<SimVisBindingsComponent>()->bindings.front()).getCamera();
}

class CameraSystem : public System, public EventListener
{
public:
	CameraSystem(const Window* visWindow, const EntityPtr& camera, const InputPlatformPtr& inputPlatform, const std::vector<LogicalAxisPtr>& axes, const sim::LatLonAltPosition& initialPosition) :
		mWindow(visWindow),
		mCamera(camera),
		mInputPlatform(inputPlatform),
		mInputAxes(axes),
		mInput(CameraController::Input::zero())
	{
		mInputPlatform->addEventListener<MouseEvent>(this);

#define PLANET_CAM
#ifdef FREE_CAM
		FreeCameraController::Params params;
		params.fovY = 0.5;
		mController.reset(new FreeCameraController(camera.get(), params));
		mController->setActive(true);
		camera->getFirstComponent<Node>()->setPosition(toGeocentric(initialPosition).position);

#else
#ifdef PLANET_CAM
		PlanetCameraController::Params params;
		params.zoomRate = 0.5;
		params.maxDistOnRadius = 7.0;
		params.fovY = 0.5f;
		mController = std::make_unique<PlanetCameraController>(camera.get(), params);
		mController->setActive(true);
		static_cast<PlanetCameraController*>(mController.get())->setLatLon(sim::LatLon(47.6062 * math::degToRadD(), -122.3321 * math::degToRadD()));
		static_cast<PlanetCameraController*>(mController.get())->setZoom(0.1);
#else
		OrbitCameraController::Params params(1, 250, 0.8);
		mController = std::make_unique<OrbitCameraController>(camera.get(), params);
		mController->setActive(true);

		OrbitCameraController* orbitCameraController = static_cast<OrbitCameraController*>(mController.get());
		orbitCameraController->setZoom(1);
		orbitCameraController->setYaw(0);
#endif
#endif
	}

	~CameraSystem()
	{
		mInputPlatform->removeEventListener(this);
	}

	void updatePostDynamics(const System::StepArgs& args) override
	{
		mInput.forwardSpeed = mInputAxes[0]->getState();
		mInput.rightSpeed = mInputAxes[1]->getState();
		mInput.panSpeed /= args.dtWallClock;
		mInput.tiltSpeed /= args.dtWallClock;
		mInput.zoomSpeed /= args.dtWallClock;
		mInput.modifierPressed = mInputPlatform->getInputDevicesOfType(InputDeviceTypeKeyboard)[0]->isButtonPressed(KC_LSHIFT);

		mController->setInput(mInput);
		mInput = CameraController::Input::zero();

		mController->update(args.dtWallClock);
		getCamera(*mCamera)->setAspectRatio((float)mWindow->getWidth() / (float)mWindow->getHeight());
	}

	CameraController* getController() const { return mController.get(); }

	void onEvent(const Event &event) override
	{
		if (const auto& mouseEvent = dynamic_cast<const MouseEvent*>(&event))
		{
			if (mouseEvent->type == MouseEvent::Type::Moved)
			{
				mInput.panSpeed += mouseEvent->relState.x;
				mInput.tiltSpeed -= mouseEvent->relState.y;
				mInput.zoomSpeed = mouseEvent->relState.z;
			}
		}
	}

private:
	std::unique_ptr<CameraController> mController;
	const Window* mWindow;
	EntityPtr mCamera;
	InputPlatformPtr mInputPlatform;
	std::vector<LogicalAxisPtr> mInputAxes;
	CameraController::Input mInput;
};

class SimVisSystem : public System
{
public:
	SimVisSystem(EngineRoot* engineRoot, const EntityPtr& simCamera) :
		mEngineRoot(engineRoot),
		mSimCamera(simCamera)
	{
		assert(mEngineRoot);
		assert(mSimCamera);

		mEngineRoot->scene->setAmbientLightColor(osg::Vec3f(0, 0, 0));
	}

	void updatePostDynamics(const System::StepArgs& args) override
	{
		mEngineRoot->scenario.timeSource.setTime(mEngineRoot->scenario.timeSource.getTime() + args.dtWallClock);

		// Calculate camera movement since last frame and apply offset to scene noise
		Vector3 cameraPosition = mSimCamera->getFirstComponent<Node>()->getPosition();

		osg::Vec3f cameraNedMovement = mCoordinateConverter.convertPosition(*getPosition(*mSimCamera));
		mEngineRoot->scene->translateNoiseOrigin(-cameraNedMovement);
		mCoordinateConverter.setOrigin(cameraPosition);

		syncVis(*mEngineRoot->simWorld, mCoordinateConverter);
	}

private:
	EngineRoot* mEngineRoot;
	EntityPtr mSimCamera;
	GeocentricToNedConverter mCoordinateConverter;
};

const float shipHeading = 0.4f;
LatLon shipLatLon(47.2 * skybolt::math::degToRadD(), -128 * skybolt::math::degToRadD()); // Ocean near Seattle
LatLonAlt shuttleLatLonAlt(35 * skybolt::math::degToRadD(), -128 * skybolt::math::degToRadD(), 620000); // Orbit
sim::LatLonAltPosition initialCameraPosition(sim::LatLonAlt(shipLatLon.lat, shipLatLon.lon, 1000));

static void createEntities(const EntityFactory& entityFactory, World& simWorld, CameraController& cameraController)
{
	// Create shuttle
	sim::EntityPtr shuttle;
	{
		auto geocentricPosition = sim::toGeocentric(sim::LatLonAltPosition(shuttleLatLonAlt));
		auto geocentricOrientation = sim::toGeocentric(sim::LtpNedOrientation(glm::angleAxis(3.5, Vector3(0, 0, 1)) * glm::angleAxis(0.3, Vector3(0, 1, 0)) * glm::angleAxis(0.35, Vector3(1, 0, 0))), toLatLon(shuttleLatLonAlt));
		shuttle = entityFactory.createEntity("Shuttle", "", geocentricPosition.position, geocentricOrientation.orientation);
		shuttle->setDynamicsEnabled(false);
		simWorld.addEntity(shuttle);
	}

	// Create scene
	EntityPtr starfield = entityFactory.createStarfield();
	simWorld.addEntity(starfield);

	EntityPtr sun = entityFactory.createSun();
	simWorld.addEntity(sun);

	EntityPtr moon = entityFactory.createMoon();
	simWorld.addEntity(moon);

	EntityPtr planet = entityFactory.createEntity("PlanetEarth");
	simWorld.addEntity(planet);

	cameraController.setTarget(planet.get());
}

static std::vector<LogicalAxisPtr> createCameraInput(const InputPlatform& inputPlatform)
{
	InputDevicePtr keyboard = inputPlatform.getInputDevicesOfType(InputDeviceTypeKeyboard)[0];
	float rate = 1000;

	std::vector<LogicalAxisPtr> logicalAxes;

	LogicalAxisPtr forwardAxis(new KeyAxis(keyboard, KC_S, KC_W, rate, rate, -1.0f, 1.0f));
	logicalAxes.push_back(forwardAxis);

	LogicalAxisPtr rightAxis(new KeyAxis(keyboard, KC_A, KC_D, rate, rate, -1.0f, 1.0f));
	logicalAxes.push_back(rightAxis);

	return logicalAxes;
}

int main(int argc, char *argv[])
{
	try
	{
		boost::program_options::options_description desc;
		EngineCommandLineParser::addOptions(desc);
		auto params = EngineCommandLineParser::parse(desc, argc, argv);
		nlohmann::json settings = skybolt::optionalMapOrElse<nlohmann::json>(EngineCommandLineParser::readSettings(params), [] { return nlohmann::json(); });

		std::string pluginsDir = getExecutablePath().append("plugins").string();
		std::vector<PluginFactory> enginePluginFactories = loadPluginFactories<Plugin, PluginConfig>(pluginsDir);

		StandaloneWindow window(RectI(10, 10, 1200, 720));
		auto engineRoot = EngineRootFactory::create(&window, enginePluginFactories, settings);

		engineRoot->scenario.timeSource.setRange(TimeRange(0, 100000));
		float timeOfDayInHours = 20;
		engineRoot->scenario.startJulianDate = calcJulianDate(2017, 8, 17, timeOfDayInHours);

		// Create camera
		EntityPtr simCamera = engineRoot->entityFactory->createEntity("Camera");
		engineRoot->simWorld->addEntity(simCamera);

		// Attach camera to window
		CameraPtr camera = getCamera(*simCamera);
		auto viewport = createAndAddViewportToWindow(window, engineRoot->programs.compositeFinal);
		viewport->setCamera(camera);
		viewport->setScene(std::make_shared<RenderTargetSceneAdapter>(engineRoot->scene));

		// Create input
		InputPlatformOisPtr inputPlatform(new InputPlatformOis(window.getHandle(), window.getWidth(), window.getHeight()));
		std::vector<LogicalAxisPtr> axes = createCameraInput(*inputPlatform);

		// Create systems
		auto cameraSystem = std::make_shared<CameraSystem>(&window, simCamera, inputPlatform, axes, initialCameraPosition);
		std::vector<std::shared_ptr<System>> systems = {
			std::make_shared<InputSystem>(inputPlatform, &window, axes),
			std::make_shared<Ships>(*engineRoot->entityFactory, *engineRoot->simWorld, shipLatLon, shipHeading),
			cameraSystem,
			std::make_shared<DebugPositionLoaderSystem>(inputPlatform.get(), simCamera->getFirstComponent<Node>().get()),
//#define SHOW_STATS // FIXME: enabling stats adds an unacceptable performance hit due to drawing the hud text
#ifdef SHOW_STATS
			std::make_shared<StatsDisplaySystem>(window, *engineRoot->scene),
#endif
			std::make_shared<SimVisSystem>(engineRoot.get(), simCamera)
		};

		// Create entities
		createEntities(*engineRoot->entityFactory, *engineRoot->simWorld, *cameraSystem->getController());

		// Run main loop
		auto simStepper = std::make_shared<SimStepper>(std::make_shared<SystemRegistry>(systems));

		double prevElapsedTime = 0;
		double minFrameDuration = 0.01;

		UpdateLoop loop(minFrameDuration);
		loop.exec([&](float dtWallClock) {
			System::StepArgs args;
			args.dtSim = dtWallClock;
			args.dtWallClock = dtWallClock;

			for (auto system : systems)
			{
				simStepper->step(args);
			}
			return window.render();
		});
	}
	catch (const std::exception& e)
	{
		printf(std::string(std::string(e.what()) + "\n").c_str());
	}

	return 0;
}
