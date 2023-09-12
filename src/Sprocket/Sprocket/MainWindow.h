/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SprocketFwd.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>
#include <SkyboltSim/SkyboltSimFwd.h>

#include <QMainWindow>
#include <QSettings>
#include <ToolWindowManager/ToolWindowManager.h>

#include <cxxtimer/cxxtimer.hpp>

class QAction;

namespace Ui { class MainWindow; }

struct MainWindowConfig
{
	std::shared_ptr<skybolt::EngineRoot> engineRoot; //!< Never null
	QWidget *parent = nullptr;
	Qt::WindowFlags flags = Qt::WindowFlags();
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(const MainWindowConfig& config);
	~MainWindow();

	skybolt::EngineRoot* getEngineRoot() const { return mEngineRoot.get(); }

	enum class OverwriteMode
	{
		PromptToSaveChanges,
		OverwriteWithoutPrompt
	};

	void clearProject();
	void openProject(const QString& filename, OverwriteMode overwriteMode = OverwriteMode::PromptToSaveChanges);
	bool saveProject(class QFile& file);
	QString getProjectFilename() const { return mProjectFilename; }

	void addToolWindow(const QString& windowName, QWidget* window);
	void raiseToolWindow(QWidget* widget);
	void restoreToolWindowsState(const QString& stateBase64);

public slots:
	void newProject(OverwriteMode overwriteMode = OverwriteMode::PromptToSaveChanges);
	void openProject();
	bool saveProject();
	bool saveProjectAs();
	void about();
	void exit();
	void editEngineSettings();

signals:
	void projectCleared();
	void projectLoaded(const nlohmann::json& json);
	void projectSaved(nlohmann::json& json) const;
	void updated(skybolt::sim::SecondsD wallDt);

private slots:
	void updateIfIntervalElapsed();

	void toolWindowActionToggled(bool state);
	void toolWindowVisibilityChanged(QWidget* toolWindow, bool visible);

protected:
	virtual bool saveChangesAndContinue(); //!< @returns action was not cancelled
	virtual void loadProject(const nlohmann::json& json);
	virtual void saveProject(nlohmann::json& json) const;
	virtual void createNewProjectEntities();

	virtual void update();

private:
	QString getDefaultProjectDirectory() const;
	void closeEvent(QCloseEvent*);

	void setProjectFilename(const QString& filename);

	void simulate(skybolt::TimeSource& timeSource, float dt);

private:
	std::shared_ptr<skybolt::EngineRoot> mEngineRoot;
	std::unique_ptr<Ui::MainWindow> ui;
	ToolWindowManager* mToolWindowManager;
	std::vector<QAction*> mToolActions;
	
	cxxtimer::Timer mUpdateTimer; //!< Time since last update

	QString mProjectFilename;

	QAction* mViewMenuToolWindowSeparator;

	QSettings mSettings;
	std::unique_ptr<class RecentFilesMenuPopulator> mRecentFilesMenuPopulator;
};
