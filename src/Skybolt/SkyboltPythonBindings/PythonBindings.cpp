/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PyComponent.h"
#include "PythonBindings.h"

#include <SkyboltCommon/Math/Box3.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/WindowUtil.h>
#include <SkyboltEngine/Components/TemplateNameComponent.h>
#include <SkyboltEngine/Components/VisObjectsComponent.h>
#include <SkyboltEngine/Scenario/ScenarioMetadataComponent.h>
#include <SkyboltEngine/SimVisBinding/CameraSimVisBinding.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/CameraController/CameraController.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/CameraController/EntityTargeter.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>
#include <SkyboltSim/Components/MainRotorComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/Spatial/Frustum.h>
#include <SkyboltSim/Spatial/GreatCircle.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltSim/System/SimStepper.h>

#include <SkyboltVis/Rect.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/Renderable/Planet/Planet.h>
#include <SkyboltVis/Renderable/Water/WaterMaterial.h>
#include <SkyboltVis/RenderOperation/DefaultRenderCameraViewport.h>
#include <SkyboltVis/RenderOperation/RenderOperationSequence.h>
#include <SkyboltVis/Window/CaptureScreenshot.h>
#include <SkyboltVis/Window/OffscreenWindow.h>
#include <SkyboltVis/Window/StandaloneWindow.h>

#include <osg/Image>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

using namespace skybolt;
using namespace skybolt::sim;

namespace skybolt {

EngineRoot* gEngineRoot = nullptr;
EngineRoot* getGlobalEngineRoot() { return gEngineRoot; }

static void setGlobalEngineRoot(EngineRoot* root) { gEngineRoot = root; 
	// FIXME: There's currently an issue where if this plugin is linked statically, calling this addStaticallyRegisteredTypes
	// will re-register previously registered types, creating duplicate types because there is only one `globalHandlers` list across statically-linked objects.
	addStaticallyRegisteredTypes(*root->typeRegistry);
}

static std::unique_ptr<EngineRoot> createEngineRootWithDefaults() {
	return EngineRootFactory::create({});
}

} // namespace skybolt

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

static void setWaveHeight(sim::Entity& entity, double height)
{
	if (vis::Planet* planet = getFirstVisObject<vis::Planet>(entity).get(); planet)
	{
		if (const auto& material = planet->getWaterMaterial(); material)
		{
			material->setWaveHeight(height);
		}
	}
}

static bool attachCameraToWindowWithEngine(sim::Entity& camera, vis::Window& window, EngineRoot& engineRoot)
{
	vis::CameraPtr visCamera = getVisCamera(camera);
	if (visCamera)
	{
		const auto& viewport = createAndAddViewportToWindowWithEngine(window, engineRoot);
		viewport->setCamera(visCamera);
		return true;
	}
	return false;
}

static void stepSim(EngineRoot& engineRoot, double dt)
{
	SimStepper stepper(engineRoot.systemRegistry);
	stepper.update(dt);
}

static bool render(EngineRoot& engineRoot, vis::VisRoot& visRoot)
{
	stepSim(engineRoot, 0.0); // FIXME: We need to call this to update all the systems prior to rendering, but we don't actually need to 'step'.
	return visRoot.render();
}

static py::array_t<std::uint8_t> captureScreenshotToImage(vis::VisRoot& visRoot)
{
	osg::ref_ptr<osg::Image> image = vis::captureScreenshot(visRoot);
	if (!image)
	{
		throw std::runtime_error("Could not capture screenshot");
	}

	const int channelCount = image->getPixelSizeInBits() / 8;
	const std::size_t size = image->s() * image->t() * channelCount;
	auto result = py::array_t<std::uint8_t>(size);

	py::buffer_info buf = result.request();
	std::uint8_t* data = static_cast<std::uint8_t*>(buf.ptr);
	std::memcpy(data, image->data(), size);

	result.resize({ image->t(), image->s(), channelCount });
    return result;
}

//! Register a python class as a component. The python class must derive from `skybolt.Component`.
static void registerComponent(EngineRoot& engineRoot, const py::handle& pyClass)
{
	auto mComponentFactoryRegistry = valueOrThrowException(getExpectedRegistry<ComponentFactoryRegistry>(*engineRoot.factoryRegistries));

	auto factory = std::make_shared<ComponentFactoryFunctionAdapter>([pyClass](Entity* entity, const ComponentFactoryContext& context, const nlohmann::json& json) {
        // Instantiate the Python class
        py::object object = pyClass(entity);

		// Create the C++ component to wrap the python object
        return std::make_shared<PyComponent>(object);
	});

	std::string componentClassName = py::str(pyClass.attr("__name__"));
	mComponentFactoryRegistry->insert(std::make_pair(componentClassName, factory));
}

PYBIND11_MAKE_OPAQUE(std::vector<std::string>);

PYBIND11_MODULE(skybolt, m) {
	py::bind_vector<std::vector<std::string>>(m, "VectorString");

	py::class_<Vector3>(m, "Vector3", "A 3D vector of [x, y, z] components")
		.def(py::init())
		.def(py::init<double, double, double>())
		.def_readwrite("x", &Vector3::x)
		.def_readwrite("y", &Vector3::y)
		.def_readwrite("z", &Vector3::z)
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self / py::self)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= double())
		.def(py::self /= double())
		.def(double() * py::self)
		.def(py::self * double())
		.def(py::self / double());

	py::class_<Quaternion>(m, "Quaternion", "Represents an orientation as a quarternion of [x, y, z, w] components")
		.def(py::init())
		.def(py::init<double, double, double, double>())
		.def_readwrite("x", &Quaternion::x)
		.def_readwrite("y", &Quaternion::y)
		.def_readwrite("z", &Quaternion::z)
		.def_readwrite("w", &Quaternion::w)
		.def(py::self * Vector3());

	py::class_<LatLon>(m, "LatLon", "Represents a 2D location on a planet location as a latitude and longitude in radians")
		.def(py::init())
		.def(py::init<double, double>())
		.def_readwrite("lat", &LatLon::lat)
		.def_readwrite("lon", &LatLon::lon);

	py::class_<LatLonAlt>(m, "LatLonAlt", "Represents a 3D location on a planet location as a latitude and longitude in radians, and altitude in meters")
		.def(py::init())
		.def(py::init<double, double, double>())
		.def_readwrite("lat", &LatLonAlt::lat)
		.def_readwrite("lon", &LatLonAlt::lon)
		.def_readwrite("alt", &LatLonAlt::alt);

	py::class_<Frustum>(m, "Frustum")
		.def(py::init())
		.def_readwrite("origin", &Frustum::origin)
		.def_readwrite("orientation", &Frustum::orientation)
		.def_readwrite("fieldOfViewHorizontal", &Frustum::fieldOfViewHorizontal)
		.def_readwrite("fieldOfViewVertical", &Frustum::fieldOfViewVertical);

	py::class_<Box3d>(m, "Box3d")
		.def(py::init())
		.def(py::init<const Vector3&, const Vector3&>())
		.def("size", &Box3d::size)
		.def("center", &Box3d::center)
		.def("merge", &Box3d::merge)
		.def_readwrite("minimum", &Box3d::minimum)
		.def_readwrite("maximum", &Box3d::maximum);
		
	py::class_<CameraState>(m, "CameraState")
		.def(py::init())
		.def_readwrite("nearClipDistance", &CameraState::nearClipDistance)
		.def_readwrite("farClipDistance", &CameraState::farClipDistance)
		.def_readwrite("fovY", &CameraState::fovY);

	py::class_<vis::RectI>(m, "RectI", "A rectangle defined by [x, y, width, height]")
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

	py::class_<EntityId, std::shared_ptr<EntityId>>(m, "EntityId")
		.def_readwrite("applicationId", &EntityId::applicationId)
		.def_readwrite("entityId", &EntityId::entityId);

	py::class_<Component, std::shared_ptr<Component>>(m, "Component", "Base class for components which can be attached to an `Entity`")
		.def(py::init<>())
		.def("setSimTime", &Component::setSimTime);

	py::class_<MainRotorComponent, std::shared_ptr<MainRotorComponent>, Component>(m, "MainRotorComponent")
		.def("getPitchAngle", &MainRotorComponent::getPitchAngle)
		.def("getRotationAngle", &MainRotorComponent::getRotationAngle)
		.def("getTppOrientationRelBody", &MainRotorComponent::getTppOrientationRelBody)
		.def("setNormalizedRpm", &MainRotorComponent::setNormalizedRpm);

	py::class_<ScenarioMetadataComponent, std::shared_ptr<ScenarioMetadataComponent>, Component>(m, "ScenarioMetadataComponent")
		.def_readwrite("serializable", &ScenarioMetadataComponent::serializable)
		.def_readwrite("deletable", &ScenarioMetadataComponent::deletable)
		.def_readwrite("directory", &ScenarioMetadataComponent::directory);

	py::class_<TemplateNameComponent, std::shared_ptr<TemplateNameComponent>, Component>(m, "TemplateNameComponent", "A component storing the name of the template which an `Entity` instantiates")
		.def_readonly("name", &TemplateNameComponent::name);

	py::class_<CameraComponent, std::shared_ptr<CameraComponent>, Component>(m, "CameraComponent")
		.def_property("state",
			[](const CameraComponent& c) { return c.getState(); },
			[](CameraComponent& c, const CameraState& state) { CameraState& s = c.getState(); s = state; },
			py::return_value_policy::reference_internal);

	py::class_<CameraControllerSelector, std::shared_ptr<CameraControllerSelector>>(m, "CameraControllerSelector")
		.def("selectController", &CameraControllerSelector::selectController)
		.def("getSelectedControllerName", &CameraControllerSelector::getSelectedControllerName)
		.def("setTargetId", &CameraControllerSelector::setTargetId);

	py::class_<CameraControllerComponent, std::shared_ptr<CameraControllerComponent>, Component, CameraControllerSelector>(m, "CameraControllerComponent");

	py::class_<EntityTargeter>(m, "EntityTargeter", "Interface for a class which references a target `Entity` by `EntityId`")
		.def("getTargetId", &EntityTargeter::getTargetId)
		.def("setTargetId", &EntityTargeter::setTargetId)
		.def("getTargetName", &EntityTargeter::getTargetName)
		.def("setTargetName", &EntityTargeter::setTargetName);

	py::class_<Entity, std::shared_ptr<Entity>>(m, "Entity")
		.def("getId", &Entity::getId)
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
		.def("removeAllEntities", &World::removeAllEntities)
		.def("findObjectByName", &World::findObjectByName);

	py::class_<EntityFactory>(m, "EntityFactory", "Class responsible for creating `Entity` instances based on a template name")
		.def("createEntity", &EntityFactory::createEntity, py::return_value_policy::reference,
			py::arg("templateName"), py::arg("name") = "", py::arg("position") = math::dvec3Zero(), py::arg("orientation") = math::dquatIdentity(), py::arg("id") = sim::nullEntityId());

	py::class_<Scenario>(m, "Scenario")
		.def_readwrite("startJulianDate", &Scenario::startJulianDate)
		.def_property("time", [](Scenario* scenario) { return scenario->timeSource.getTime(); }, [](Scenario* scenario, double time) { return scenario->timeSource.setTime(time); })
		.def_property_readonly("currentJulianDate", [](Scenario* scenario) {return getCurrentJulianDate(*scenario); });

	py::class_<EngineRoot>(m, "EngineRoot")
		.def_property_readonly("world", [](const EngineRoot& r) {return &r.scenario->world; }, py::return_value_policy::reference_internal)
		.def_property_readonly("entityFactory", [](const EngineRoot& r) {return r.entityFactory.get(); }, py::return_value_policy::reference_internal)
		.def_property_readonly("scenario", [](const EngineRoot& r) {return r.scenario.get(); }, py::return_value_policy::reference_internal)
		.def("locateFile", [](const EngineRoot& r, const std::string& filename) { return value(r.fileLocator(filename)).value_or("").string(); });

	py::class_<vis::Window, std::shared_ptr<vis::Window>>(m, "Window");

	py::class_<vis::StandaloneWindow, std::shared_ptr<vis::StandaloneWindow>, vis::Window>(m, "StandaloneWindow")
		.def(py::init<vis::RectI>());

	py::class_<vis::OffscreenWindow, std::shared_ptr<vis::OffscreenWindow>, vis::Window>(m, "OffscreenWindow")
		.def(py::init<int, int>()); // width, height

	py::enum_<vis::LoadTimingPolicy>(m, "LoadTimingPolicy", "Enum specifying policy for timing of loading data into the renderer")
    .value("LoadAcrossMultipleFrames", vis::LoadTimingPolicy::LoadAcrossMultipleFrames)
    .value("LoadBeforeRender", vis::LoadTimingPolicy::LoadBeforeRender)
    .export_values();

	py::class_<vis::VisRoot>(m, "VisRoot")
		.def(py::init())
		.def("addWindow", &vis::VisRoot::addWindow)
		.def("removeWindow", &vis::VisRoot::removeWindow)
		.def("setLoadTimingPolicy", &vis::VisRoot::setLoadTimingPolicy);

	m.def("getGlobalEngineRoot", &getGlobalEngineRoot, "Get global EngineRoot", py::return_value_policy::reference);
	m.def("setGlobalEngineRoot", &setGlobalEngineRoot, "Set global EngineRoot");
	m.def("createEngineRootWithDefaults", &createEngineRootWithDefaults, "Create an EngineRoot with default values");
	m.def("attachCameraToWindowWithEngine", &attachCameraToWindowWithEngine);
	m.def("registerComponent", &registerComponent);
	m.def("stepSim", &stepSim);
	m.def("render", &render, py::arg("engineRoot"), py::arg("window"));
	m.def("toGeocentricPosition", [](const PositionPtr& position) { return std::make_shared<GeocentricPosition>(toGeocentric(*position)); });
	m.def("toGeocentricOrientation", [](const OrientationPtr& orientation, const LatLon& latLon) { return std::make_shared<GeocentricOrientation>(toGeocentric(*orientation, latLon)); });
	m.def("toLatLonAlt", [](const PositionPtr& position) { return std::make_shared<LatLonAltPosition>(toLatLonAlt(*position)); });
	m.def("toLatLon", &toLatLon);
	m.def("toLatLonAlt", py::overload_cast<const LatLon&, double>(&toLatLonAlt));
	m.def("dot", &dotFunc);
	m.def("cross", &crossFunc);
	m.def("normalize", &normalizeFunc);
	m.def("quaternionFromEuler", py::overload_cast<const Vector3&>(&math::quatFromEuler));
	m.def("captureScreenshot", [](vis::VisRoot& visRoot) { return captureScreenshotToImage(visRoot); });
	m.def("captureScreenshot", [](vis::VisRoot& visRoot, const std::string& filename) { return vis::captureScreenshot(visRoot, filename); });
	m.def("moveDistanceAndBearing", &moveDistanceAndBearing);
	m.def("transformToScreenSpace", &transformToScreenSpace);
	m.def("setWaveHeight", &setWaveHeight);
	m.def("calcSmallestAngleFromTo", &math::calcSmallestAngleFromTo<double>);
}
