/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/Chrono.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <nlohmann/json.hpp>
#include <optional>
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
	std::optional<ErrorMessage> saveScenario(QFile& file);

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

void connectJsonScenarioSerializable(ScenarioWorkspace& workspace, JsonScenarioSerializable& serializable);
