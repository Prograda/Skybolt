#include "ScenarioWorkspace.h"

#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Json/WriteJsonFile.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltSim/Components/CameraControllerComponent.h>

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

using namespace skybolt;

ScenarioWorkspace::ScenarioWorkspace(const skybolt::NonNullPtr<skybolt::EngineRoot>& engineRoot, ScenarioSetupFunction scenarioSetupFunction) :
	mScenarioSetupFunction(scenarioSetupFunction),
	mEngineRoot(engineRoot)
{
	assert(mScenarioSetupFunction);
	assert(mEngineRoot);

	newScenario();
}

void ScenarioWorkspace::newScenario()
{
	unloadScenario();
	mScenarioSetupFunction(mEngineRoot->scenario->world, *mEngineRoot->entityFactory);

	emit scenarioNewed();
}

std::optional<ScenarioWorkspace::ErrorMessage> ScenarioWorkspace::loadScenario(const QString& filename)
{
	if (!QFileInfo::exists(filename))
	{
		return "Could not open file '" + filename + "' because it does not exist";
	}

	QFile file(filename);
	return loadScenario(file);
}

std::optional<ScenarioWorkspace::ErrorMessage> ScenarioWorkspace::loadScenario(QFile& file)
{
	if (!file.isOpen())
	{
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			return "Could not open file '" + file.fileName() + "'. " + file.errorString();
		}
	}

	unloadScenario();

	try
	{
		std::string content = file.readAll().toStdString();
		nlohmann::json json = nlohmann::json::parse(content);

		loadScenario(json);
		setScenarioFilename(file.fileName());
	}
	catch (std::exception& e)
	{
		newScenario();
		return e.what();
	}
	return std::nullopt;
}

std::optional<ScenarioWorkspace::ErrorMessage> ScenarioWorkspace::saveScenario(const QString& filename)
{
	QSaveFile file(filename);
	if (!file.open(QIODevice::WriteOnly))
	{
		throw std::runtime_error("Could not open file '" + filename.toStdString() + "' for writing. " + file.errorString().toStdString());
	}

	setScenarioFilename(file.fileName());

	nlohmann::json json;
	saveScenario(json);

	int indent = 4;
	file.write(json.dump(indent).c_str());
	file.commit();

	return std::nullopt;
}

void ScenarioWorkspace::createDefaultNewScenarioEntities(sim::World& world, EntityFactory& factory)
{
	// Create earth
	auto planet = factory.createEntity("PlanetEarth");
	world.addEntity(planet);

	// Create camera
	sim::EntityPtr camera = factory.createEntity("Camera");
	camera->getFirstComponentRequired<sim::CameraControllerComponent>()->setTargetId(planet->getId());
	world.addEntity(camera);
}

//! Create objects that are present in every scenario
void createBackgroundEntities(sim::World& world, const EntityFactory& factory)
{
	world.addEntity(factory.createEntity("Stars"));
	world.addEntity(factory.createEntity("SunBillboard"));
	world.addEntity(factory.createEntity("MoonBillboard"));
}

void ScenarioWorkspace::unloadScenario()
{
	setScenarioFilename("");

	mEngineRoot->scenario->world.removeAllEntities();
	createBackgroundEntities(mEngineRoot->scenario->world, *mEngineRoot->entityFactory);

	emit scenarioUnloaded();
}

void ScenarioWorkspace::loadScenario(const nlohmann::json& json)
{
	EntityFactoryFn entityFactoryFn = [entityFactory = mEngineRoot->entityFactory.get()](const std::string& templateName, const std::string& instanceName) {
		return entityFactory->createEntity(templateName, instanceName);
	};

	ifChildExists(json, "scenario", [this, entityFactoryFn] (const nlohmann::json& child) {
		readScenario(*mEngineRoot->typeRegistry, *mEngineRoot->scenario, entityFactoryFn, child);
	});

	emit scenarioLoaded(json);
}

void ScenarioWorkspace::saveScenario(nlohmann::json& json) const
{
	json["scenario"] = writeScenario(*mEngineRoot->typeRegistry, *mEngineRoot->scenario);
	emit scenarioSaved(json);
}

void ScenarioWorkspace::setScenarioFilename(const QString& filename)
{
	mScenarioFilename = filename;
	emit scenarioFilenameChanged(filename);
}
