/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQtFwd.h"
#include "Scenario/JsonScenarioSerializable.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>

#include <QMainWindow>
#include <QSettings>
#include <ToolWindowManager/ToolWindowManager.h>

class QAction;

namespace Ui { class MainWindow; }

struct MainWindowConfig
{
	ScenarioWorkspacePtr workspace;
	std::shared_ptr<skybolt::EngineRoot> engineRoot; //!< Never null
	QWidget *parent = nullptr;
	Qt::WindowFlags flags = Qt::WindowFlags();
};

class MainWindow : public QMainWindow, public JsonScenarioSerializable
{
	Q_OBJECT

public:
	MainWindow(const MainWindowConfig& config);
	~MainWindow();

	skybolt::EngineRoot* getEngineRoot() const { return mEngineRoot.get(); }

	void addToolWindow(const QString& windowName, QWidget* window);
	void raiseToolWindow(QWidget* widget);
	void restoreToolWindowsState(const QString& stateBase64);

	enum class OverwriteMode
	{
		PromptToSaveChanges,
		OverwriteWithoutPrompt
	};

	virtual void newScenario(OverwriteMode mode);
	virtual void openScenario(const QString& filename, OverwriteMode mode);

public: // JsonScenarioSerializable interface
	void readScenario(const nlohmann::json& json) override;
	void writeScenario(nlohmann::json& json) const override;

public slots:
	void newScenario();
	void openScenario();
	bool saveScenario();
	bool saveScenarioAs();
	void about();
	void exit();
	void editEngineSettings();
	void scenarioFilenameChanged(const QString& filename);

private slots:
	void toolWindowActionToggled(bool state);
	void toolWindowVisibilityChanged(QWidget* toolWindow, bool visible);

protected:
	virtual bool saveChangesAndContinue(); //!< @returns action was not cancelled

	virtual void update();

private:
	QString getDefaultScenarioDirectory() const;
	void closeEvent(QCloseEvent*);

private:
	ScenarioWorkspacePtr mWorkspace;
	std::shared_ptr<skybolt::EngineRoot> mEngineRoot;
	std::unique_ptr<Ui::MainWindow> ui;
	ToolWindowManager* mToolWindowManager;
	std::vector<QAction*> mToolActions;
	
	QAction* mViewMenuToolWindowSeparator;

	QSettings mSettings;
	std::unique_ptr<class RecentFilesMenuPopulator> mRecentFilesMenuPopulator;
};
