/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/Scenario/JsonScenarioSerializable.h"
#include "SkyboltQt/SkyboltQtFwd.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/Chrono.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <nlohmann/json.hpp>
#include <optional>
#include <type_traits>
#include <QObject>
#include <QString>

class QFile;

class ScenarioWorkspace : public QObject
{
	Q_OBJECT
public:
	using ScenarioSetupFunction = std::function<void(skybolt::sim::World& world, skybolt::EntityFactory& factory)>;
	ScenarioWorkspace(const std::shared_ptr<skybolt::EngineRoot>& engineRoot, ScenarioSetupFunction scenarioSetupFunction = &createDefaultNewScenarioEntities);
	virtual ~ScenarioWorkspace() = default;

	using ErrorMessage = QString;
	
	void newScenario();

	std::optional<ErrorMessage> loadScenario(const QString& filename);
	std::optional<ErrorMessage> loadScenario(QFile& file);
	std::optional<ErrorMessage> saveScenario(const QString& filename);

	QString getScenarioFilename() const { return mScenarioFilename; }

	static void createDefaultNewScenarioEntities(skybolt::sim::World& world, skybolt::EntityFactory& factory);

signals:
	void scenarioUnloaded();
	void scenarioNewed();
	void scenarioLoaded(const nlohmann::json& json);
	void scenarioSaved(nlohmann::json& json) const;
	void scenarioFilenameChanged(const QString& filename);

protected:
	virtual void unloadScenario();
	virtual void loadScenario(const nlohmann::json& json);
	virtual void saveScenario(nlohmann::json& json) const;

	virtual void setScenarioFilename(const QString& filename);

private:
	ScenarioSetupFunction mScenarioSetupFunction;
	std::shared_ptr<skybolt::EngineRoot> mEngineRoot;
	QString mScenarioFilename;
};

template <class SerializableT>
void connectJsonScenarioSerializable(ScenarioWorkspace& workspace, SerializableT* serializable)
{
	static_assert(std::is_base_of<JsonScenarioSerializable, SerializableT>::value, "Type must be a JsonScenarioSerializable");
	static_assert(std::is_base_of<QObject, SerializableT>::value, "Type must be a QObject");

	QObject::connect(&workspace, &ScenarioWorkspace::scenarioNewed, serializable, [serializable] { serializable->resetScenario(); });
	QObject::connect(&workspace, &ScenarioWorkspace::scenarioLoaded, serializable, [serializable] (const nlohmann::json& json) { serializable->readScenario(json); });
	QObject::connect(&workspace, &ScenarioWorkspace::scenarioSaved, serializable, [serializable] (nlohmann::json& json) { serializable->writeScenario(json); });
}
