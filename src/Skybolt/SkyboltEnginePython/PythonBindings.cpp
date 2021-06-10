/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/CameraController/CameraController.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/OrbitComponent.h>
#include <SkyboltSim/Components/MainRotorComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Components/ParentReferenceComponent.h>
#include <SkyboltSim/Components/ProceduralLifetimeComponent.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltSim/System/SimStepper.h>

#include <SkyboltVis/Rect.h>
#include <SkyboltVis/RenderTarget/RenderTargetSceneAdapter.h>
#include <SkyboltVis/RenderTarget/ViewportHelpers.h>
#include <SkyboltVis/Window/StandaloneWindow.h>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace skybolt;
using namespace skybolt::sim;

EngineRoot* gEngineRoot = nullptr;
static EngineRoot* getGlobalEngineRoot() { return gEngineRoot; }
static void setGlobalEngineRoot(EngineRoot* root) { gEngineRoot = root; }

static std::unique_ptr<EngineRoot> createEngineRootWithDefaults() {
	return EngineRootFactory::create({});
}

static void removeNamespaceQualifier(std::string& str)
{
	size_t p = str.find_last_of(":");
	if (p != std::string::npos)
	{
		str = str.substr(p + 1);
	}
}

static std::vector<ComponentPtr> getComponentsOfTypeName(Entity* entity, const std::string& typeName)
{
	std::vector<ComponentPtr> result;

	std::vector<ComponentPtr> components = entity->getComponents();
	for (const ComponentPtr& component : components)
	{
		std::string name(typeid(*component).name());
		py::detail::clean_type_id(name);
		removeNamespaceQualifier(name);
		if (name == typeName)
		{
			result.push_back(component);
		}
	}
	return result;
};

static ComponentPtr getFirstComponentOfTypeName(Entity* entity, const std::string& typeName)
{
	const auto v = getComponentsOfTypeName(entity, typeName);
	return v.empty() ? nullptr : v.front();
}

static double dotFunc(const Vector3& a, const Vector3& b)
{
	return glm::dot(a, b);
}

static Vector3 crossFunc(const Vector3& a, const Vector3& b)
{
	return glm::cross(a, b);
}

static Vector3 normalizeFunc(const Vector3& v)
{
	return glm::normalize(v);
}

static bool attachCameraToWindowWithEngine(sim::Entity& camera, vis::Window& window, EngineRoot& engineRoot)
{
	auto viewport = createAndAddViewportToWindow(window, engineRoot.programs.getRequiredProgram("compositeFinal"));
	viewport->setScene(std::make_shared<vis::RenderTargetSceneAdapter>(engineRoot.scene));
	vis::CameraPtr visCamera = getVisCamera(camera);
	if (visCamera)
	{
		viewport->setCamera(visCamera);
		return true;
	}

	return false;
}

static bool stepOnceAndRenderOnce(EngineRoot& engineRoot, vis::Window& window, double dtWallClock)
{
	SimStepper stepper(engineRoot.systemRegistry);
	System::StepArgs args;
	args.dtSim = dtWallClock;
	args.dtWallClock = dtWallClock;
	stepper.step(args);
	return window.render();
}

static bool stepOnceAndRenderUntilDone(EngineRoot& engineRoot, vis::Window& window, double dtWallClock)
{
	SimStepper stepper(engineRoot.systemRegistry);
	System::StepArgs args;
	args.dtSim = dtWallClock;
	args.dtWallClock = dtWallClock;
	stepper.step(args);

	bool result = window.render();
	while (engineRoot.stats.tileLoadQueueSize > 0)
	{
		result = window.render();
	}
	// Render a second time in case something finished loading before we checked the queue size.
	// FIXME: Make 'done' detection more robust.
	result = window.render();
	return result;
}

PYBIND11_MODULE(skybolt, m) {
	py::class_<Vector3>(m, "Vector3")
		.def(py::init())
		.def(py::init<double, double, double>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z)
		.def(py::self + py::self)
		.def(py::self += py::self)
		.def(py::self *= double())
		.def(double() * py::self);

	py::class_<Quaternion>(m, "Quaternion")
		.def(py::init())
		.def(py::init<double, double, double, double>())
		.def_readwrite("x", &Quaternion::x)
		.def_readwrite("y", &Quaternion::y)
		.def_readwrite("z", &Quaternion::z)
		.def_readwrite("w", &Quaternion::w);

	py::class_<LatLon>(m, "LatLon")
		.def(py::init())
		.def(py::init<double, double>())
		.def_readwrite("lat", &LatLon::lat)
		.def_readwrite("lon", &LatLon::lon);

	py::class_<LatLonAlt>(m, "LatLonAlt")
		.def(py::init())
		.def(py::init<double, double, double>())
		.def_readwrite("lat", &LatLonAlt::lat)
		.def_readwrite("lon", &LatLonAlt::lon)
		.def_readwrite("alt", &LatLonAlt::alt);

	py::class_<Orbit>(m, "Orbit")
		.def(py::init())
		.def_readwrite("semiMajorAxis", &Orbit::semiMajorAxis)
		.def_readwrite("eccentricity", &Orbit::eccentricity)
		.def_readwrite("inclination", &Orbit::inclination)
		.def_readwrite("rightAscension", &Orbit::rightAscension)
		.def_readwrite("argumentOfPeriapsis", &Orbit::argumentOfPeriapsis)
		.def_readwrite("trueAnomaly", &Orbit::trueAnomaly);
		

	py::class_<vis::RectI>(m, "RectI")
		.def(py::init<int, int, int, int>());

	py::class_<Position, std::shared_ptr<Position>>(m, "Position");

	py::class_<GeocentricPosition, std::shared_ptr<GeocentricPosition>, Position>(m, "GeocentricPosition")
		.def(py::init<Vector3>())
		.def_readwrite("position", &GeocentricPosition::position);

	py::class_<LatLonAltPosition, std::shared_ptr<LatLonAltPosition>, Position>(m, "LatLonAltPosition")
		.def(py::init<LatLonAlt>())
		.def_readwrite("position", &LatLonAltPosition::position);

	py::class_<Orientation, std::shared_ptr<Orientation>>(m, "Orientation");

	py::class_<GeocentricOrientation, std::shared_ptr<GeocentricOrientation>, Orientation>(m, "GeocentricOrientation")
		.def(py::init<Quaternion>())
		.def_readwrite("orientation", &GeocentricOrientation::orientation);

	py::class_<LtpNedOrientation, std::shared_ptr<LtpNedOrientation>, Orientation>(m, "LtpNedOrientation")
		.def(py::init<Quaternion>())
		.def_readwrite("orientation", &LtpNedOrientation::orientation);

	py::class_<Component, std::shared_ptr<Component>>(m, "Component");

	py::class_<OrbitComponent, std::shared_ptr<OrbitComponent>, Component>(m, "OrbitComponent")
		.def(py::init())
		.def_readwrite("orbit", &OrbitComponent::orbit);

	py::class_<MainRotorComponent, std::shared_ptr<MainRotorComponent>, Component>(m, "MainRotorComponent")
		.def("getPitchAngle", &MainRotorComponent::getPitchAngle)
		.def("getRotationAngle", &MainRotorComponent::getRotationAngle)
		.def("getTppOrientationRelBody", &MainRotorComponent::getTppOrientationRelBody)
		.def("setNormalizedRpm", &MainRotorComponent::setNormalizedRpm);

	py::class_<ParentReferenceComponent, std::shared_ptr<ParentReferenceComponent>, Component>(m, "ParentReferenceComponent")
		.def(py::init<sim::Entity*>())
		.def("getParent", &ParentReferenceComponent::getParent);

	py::class_<CameraControllerComponent, std::shared_ptr<CameraControllerComponent>, Component>(m, "CameraControllerComponent")
		.def_property_readonly("cameraController", [](const CameraControllerComponent& c) {return c.cameraController.get(); }, py::return_value_policy::reference_internal);

	py::class_<ProceduralLifetimeComponent, std::shared_ptr<ProceduralLifetimeComponent>, Component>(m, "ProceduralLifetimeComponent")
		.def(py::init());

	py::class_<CameraController>(m, "CameraController")
		.def("getTarget", &CameraController::getTarget)
		.def("setTarget", &CameraController::setTarget);

	py::class_<CameraControllerSelector, CameraController>(m, "CameraControllerSelector")
		.def("selectController", &CameraControllerSelector::selectController)
		.def("getSelectedControllerName", &CameraControllerSelector::getSelectedControllerName);

	py::class_<Entity, std::shared_ptr<Entity>>(m, "Entity")
		.def("getName", [](Entity* entity) { return getName(*entity); })
		.def("getPosition", [](Entity* entity) {
			return *getPosition(*entity);
		})
		.def("setPosition", [](Entity* entity, const Vector3& position) {
			setPosition(*entity, position);
		})
		.def("getOrientation", [](Entity* entity) {
			return *getOrientation(*entity);
		})
		.def("setOrientation", [](Entity* entity, const Quaternion& orientation) {
			setOrientation(*entity, orientation);
		})
		.def("getComponents", &Entity::getComponents)
		.def("getComponentsOfType", &getComponentsOfTypeName)
		.def("getFirstComponentOfType", &getFirstComponentOfTypeName)
		.def("addComponent", &Entity::addComponent)
		.def_property("dynamicsEnabled", &Entity::isDynamicsEnabled, &Entity::setDynamicsEnabled);

	py::class_<World>(m, "World")
		.def("getEntities", &World::getEntities, py::return_value_policy::reference)
		.def("addEntity", &World::addEntity)
		.def("removeEntity", &World::removeEntity)
		.def("removeAllEntities", &World::removeAllEntities);

	py::class_<EntityFactory>(m, "EntityFactory")
		.def("createEntity", &EntityFactory::createEntity, py::return_value_policy::reference,
			py::arg("templateName"), py::arg("name") = "", py::arg("position") = math::dvec3Zero(), py::arg("orientation") = math::dquatIdentity());

	py::class_<EngineRoot>(m, "EngineRoot")
		.def_property_readonly("world", [](const EngineRoot& r) {return r.simWorld.get(); }, py::return_value_policy::reference_internal)
		.def_property_readonly("entityFactory", [](const EngineRoot& r) {return r.entityFactory.get(); }, py::return_value_policy::reference_internal);

	py::class_<vis::Window>(m, "Window");

	py::class_<vis::StandaloneWindow, vis::Window>(m, "StandaloneWindow")
		.def(py::init<vis::RectI>());

	m.def("getGlobalEngineRoot", &getGlobalEngineRoot, "Get global EngineRoot", py::return_value_policy::reference);
	m.def("setGlobalEngineRoot", &setGlobalEngineRoot, "Set global EngineRoot");
	m.def("createEngineRootWithDefaults", &createEngineRootWithDefaults, "Create an EngineRoot with default values");
	m.def("attachCameraToWindowWithEngine", &attachCameraToWindowWithEngine);
	m.def("stepOnceAndRenderOnce", &stepOnceAndRenderOnce);
	m.def("stepOnceAndRenderUntilDone", &stepOnceAndRenderUntilDone);
	m.def("toGeocentricPosition", [](const PositionPtr& position) { return std::make_shared<GeocentricPosition>(toGeocentric(*position)); });
	m.def("toGeocentricOrientation", [](const OrientationPtr& orientation, const LatLon& latLon) { return std::make_shared<GeocentricOrientation>(toGeocentric(*orientation, latLon)); });
	m.def("toLatLonAlt", [](const PositionPtr& position) { return std::make_shared<LatLonAltPosition>(toLatLonAlt(*position)); });
	m.def("toLatLon", &toLatLon);
	m.def("dot", &dotFunc);
	m.def("cross", &crossFunc);
	m.def("normalize", &normalizeFunc);
	m.def("quaternionFromEuler", py::overload_cast<const Vector3&>(&math::quatFromEuler));
}
