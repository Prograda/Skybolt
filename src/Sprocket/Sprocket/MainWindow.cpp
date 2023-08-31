/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "EditorPlugin.h"
#include "EngineSettingsSerialization.h"
#include "RecentFilesMenuPopulator.h"
#include "QtUtil/QtDialogUtil.h"
#include "Property/PropertyEditor.h"
#include "Widgets/SettingsEditor.h"

#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Json/WriteJsonFile.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/EngineStats.h>
#include <SkyboltEngine/Scenario/Scenario.h>
#include <SkyboltEngine/Scenario/ScenarioSerialization.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltSim/Physics/Astronomy.h>
#include <SkyboltSim/System/SimStepper.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltSim/System/SystemRegistry.h>
#include <SkyboltVis/Camera.h>

#include <filesystem>
#include <boost/log/trivial.hpp>

#include <ToolWindowManager/ToolWindowManagerArea.h>

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QToolBar>

#include <osgDB/Registry>

#include <algorithm>
#include <chrono>
#include <sstream>

using namespace skybolt;
using namespace skybolt::sim;

MainWindow::MainWindow(const MainWindowConfig& windowConfig) :
	QMainWindow(windowConfig.parent, windowConfig.flags),
	ui(new Ui::MainWindow),
	mEngineRoot(windowConfig.engineRoot),
	mSettings(QApplication::applicationName())
{
	assert(mEngineRoot);

	RecentFilesMenuPopulator::FileOpener fileOpener = [this](const QString& filename) {
		openProject(filename, OverwriteMode::PromptToSaveChanges);
	};

	ui->setupUi(this);
	mViewMenuToolWindowSeparator = ui->menuView->addSeparator();
	mRecentFilesMenuPopulator.reset(new RecentFilesMenuPopulator(ui->menuRecentFiles, &mSettings, fileOpener));

	mSimStepper = std::make_unique<SimStepper>(mEngineRoot->systemRegistry);

	setProjectFilename("");
	
	// Connect menus
	QObject::connect(ui->actionNewScenario, SIGNAL(triggered()), this, SLOT(newProject()));
	QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openProject()));
	QObject::connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveProject()));
	QObject::connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveProjectAs()));
	QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(exit()));
	QObject::connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(editEngineSettings()));

	mToolWindowManager = new ToolWindowManager(this);
	// setup tool window manager
	connect(mToolWindowManager, SIGNAL(toolWindowVisibilityChanged(QWidget*, bool)), this, SLOT(toolWindowVisibilityChanged(QWidget*, bool)));

	setCentralWidget(mToolWindowManager);

	setStatusBar(new QStatusBar(this));

	float timeOfDayInHours = 20;
	mEngineRoot->scenario->startJulianDate = calcJulianDate(2017, 8, 17, timeOfDayInHours);

	clearProject();

	// Update immediately to sync ths vis to the sim.
	// Necessary because Qt may decide to render the viewport before the update timer fires for the first time.
	update();

	QTimer::singleShot(0, this, SLOT(updateIfIntervalElapsed()));
}

MainWindow::~MainWindow()
{
	delete mToolWindowManager;

	mSimStepper.reset();
	mEngineRoot.reset();
}

double minFrameDuration = 0.01;

void MainWindow::simulate(TimeSource& timeSource, float dt)
{
	if (timeSource.getTime() == 0)
	{
		mLatestSimTime = timeSource.getTime();
	}

	timeSource.update(dt);
	double dtSim = std::max(0.0, timeSource.getTime() - mLatestSimTime);
	if (dtSim > 0.0)
	{
		mLatestSimTime = timeSource.getTime();
	}
	System::StepArgs args;
	args.dtSim = dtSim;
	args.dtWallClock = dt;
	mSimStepper->step(args);
}

static QString addSeparator(const QString& str)
{
	QString result = str;
	if (!str.isEmpty())
	{
		result += ", ";
	}
	return result;
}

void MainWindow::update()
{
	double dt = double(mUpdateTimer.count<std::chrono::milliseconds>()) / 1000.0;
	mUpdateTimer.reset();
	mUpdateTimer.start();

	mEngineRoot->scenario->timeSource.update(dt);

	simulate(mEngineRoot->scenario->timeSource, dt);

	emit updated();

	QString status;
	if (mEngineRoot->stats.terrainTileLoadQueueSize)
	{
		status += "Loading terrain tiles: " + QString::number(mEngineRoot->stats.terrainTileLoadQueueSize);
	}

	if (mEngineRoot->stats.featureTileLoadQueueSize)
	{
		status = addSeparator(status);
		status += "Loading feature tiles: " + QString::number(mEngineRoot->stats.featureTileLoadQueueSize);
	}

	uint32_t activeTasks = mEngineRoot->scheduler->active_threads();
	if (activeTasks)
	{
		status = addSeparator(status);
		status += "Processing tasks: " + QString::number(activeTasks);
	}
	statusBar()->showMessage(status);
}

void MainWindow::updateIfIntervalElapsed()
{
	QTimer::singleShot(0, this, SLOT(updateIfIntervalElapsed()));

	double elapsed = double(mUpdateTimer.count<std::chrono::milliseconds>()) / 1000.0;
	if (elapsed < minFrameDuration)
	{
		return;
	}

	update();
}

static QVariantMap toVariantMap(const QByteArray& array)
{
	QVariantMap variantMap;
	QDataStream stream(const_cast<QByteArray*>(&array), QIODevice::OpenMode::enum_type::ReadOnly);
	stream >> variantMap;
	return variantMap;
}

#define FOREACH_CALL(container, fn, ...) \
for (const auto& __item__ : container) \
	__item__->fn(__VA_ARGS__);

void MainWindow::clearProject()
{
	setProjectFilename("");

	mEngineRoot->scenario->world.removeAllEntities();

	emit projectCleared();
}

void MainWindow::restoreToolWindowsState(const QString& stateBase64)
{
	mToolWindowManager->restoreState(toVariantMap(QByteArray::fromBase64(stateBase64.toUtf8())));
}

void createDefaultObjects(World& world, const EntityFactory& factory)
{
	world.addEntity(factory.createEntity("Stars"));
	world.addEntity(factory.createEntity("SunBillboard"));
	world.addEntity(factory.createEntity("MoonBillboard"));
}

bool MainWindow::saveChangesAndContinue()
{
	QMessageBox msgBox(this);
	msgBox.setText("Do you want to save changes?");
	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Save);

	switch (msgBox.exec())
	{
		case QMessageBox::Save:
			return saveProject();
		case QMessageBox::Discard:
			return true;
		case QMessageBox::Cancel:
			return false;
		default:
			assert(!"Should never get here");
	}
	return true;
}

void MainWindow::newProject(OverwriteMode overwriteMode)
{
	if (overwriteMode == OverwriteMode::PromptToSaveChanges && !saveChangesAndContinue())
	{
		return;
	}

	const EntityFactory& factory = *mEngineRoot->entityFactory;
	World& simWorld = mEngineRoot->scenario->world;

	clearProject();

	// Create camera
	sim::EntityPtr camera = factory.createEntity("Camera");
	simWorld.addEntity(camera);

	createDefaultObjects(simWorld, factory);

	// Create earth
	auto planet = factory.createEntity("PlanetEarth");
	simWorld.addEntity(planet);
}

QString projectFileExtension = ".proj";
QString projectFileFilter = "Project Files (*" + projectFileExtension + ")";

QString defaultDirKey = "defaultDir";

QString MainWindow::getDefaultProjectDirectory() const
{
	QString defaultDir = mSettings.value(defaultDirKey).toString();
	if (defaultDir.isEmpty())
	{
		defaultDir = QDir::homePath();
	}
	return defaultDir;
}

void MainWindow::openProject()
{
	if (!saveChangesAndContinue())
	{
		return;
	}

	QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Project"),
		getDefaultProjectDirectory(), projectFileFilter);

	if (!filename.isEmpty())
	{
		QDir currentDir;
		mSettings.setValue(defaultDirKey, currentDir.absoluteFilePath(filename));

		openProject(filename, OverwriteMode::OverwriteWithoutPrompt); // the user has already been prompted to overwrite
	}
}

static QByteArray toByteArray(const QVariantMap& variant)
{
	QByteArray array;
	QDataStream stream(&array, QIODevice::OpenMode::enum_type::WriteOnly);
	stream << variant;
	return array;
}

void MainWindow::openProject(const QString& filename, OverwriteMode overwriteMode)
{
	if (overwriteMode == OverwriteMode::PromptToSaveChanges && !saveChangesAndContinue())
	{
		return;
	}

	if (!QFileInfo::exists(filename))
	{
		QMessageBox::critical(this, "", "Could not open file because it does not exist");
		return;
	}

	QFile file(filename);

	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::critical(this, "", "Could not open file. " + file.errorString());
		return;
	}

	clearProject();
	createDefaultObjects(mEngineRoot->scenario->world, *mEngineRoot->entityFactory);

	setProjectFilename(filename);

	try
	{
		std::string content = file.readAll().toStdString();
		nlohmann::json json = nlohmann::json::parse(content);

		loadProject(json);
		mRecentFilesMenuPopulator->addFilename(filename);
	}
	catch (std::exception &e)
	{
		QMessageBox::critical(this, "Error", e.what());
		clearProject();
	}
}

void MainWindow::loadProject(const nlohmann::json& json)
{
	ifChildExists(json, "scenario", [this] (const nlohmann::json& child) {
		readScenario(*mEngineRoot->scenario, child);
	});

	ifChildExists(json, "mainWindowGeometry", [this] (const nlohmann::json& child) {
		QString str = QString::fromStdString(child.get<std::string>());
		restoreGeometry(QByteArray::fromBase64(str.toUtf8()));
	});

	ifChildExists(json, "windowLayout", [this] (const nlohmann::json& child) {
		QString str = QString::fromStdString(child.get<std::string>());
		mToolWindowManager->restoreState(toVariantMap(QByteArray::fromBase64(str.toUtf8())));
	});

	ifChildExists(json, "entities", [this] (const nlohmann::json& child) {
		readEntities(mEngineRoot->scenario->world, *mEngineRoot->entityFactory, child);
	});

	emit projectLoaded(json);
}

bool MainWindow::saveProject(QFile& file)
{
	if (file.open(QIODevice::WriteOnly))
	{
		nlohmann::json json;
		saveProject(json);

		int indent = 4;
		file.write(json.dump(indent).c_str());
		return true;
	}
	else
	{
		QMessageBox::critical(this, "", "Could not write to file. " + file.errorString());
		return false;
	}
}

void MainWindow::saveProject(nlohmann::json& json) const
{
	json["scenario"] = writeScenario(*mEngineRoot->scenario);
	json["windowLayout"] = toByteArray(mToolWindowManager->saveState()).toBase64().toStdString();
	json["mainWindowGeometry"] = saveGeometry().toBase64().toStdString();
	json["entities"] = writeEntities(mEngineRoot->scenario->world);

	emit projectSaved(json);
}

bool MainWindow::saveProject()
{
	if (mProjectFilename.isEmpty())
	{
		saveProjectAs();
	}
	else
	{
		QFile file(mProjectFilename);
		return saveProject(file);
	}
	return false;
}

bool MainWindow::saveProjectAs()
{
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save Project"),
		getDefaultProjectDirectory(), projectFileFilter);

	if (!fileName.isEmpty())
	{
		if (!fileName.endsWith(projectFileExtension, Qt::CaseInsensitive))
			fileName += projectFileExtension;

		QFile file(fileName);
		if (!saveProject(file))
		{
			return false;
		}
		setProjectFilename(fileName);

		QDir currentDir;
		mSettings.setValue(defaultDirKey, currentDir.absoluteFilePath(fileName));
		return true;
	}
	return false;
}

void MainWindow::about()
{
	QMessageBox::about(this, QApplication::applicationName(), "Copyright 2023 Matthew Paul Reid");
}

void MainWindow::exit()
{
	close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	event->ignore();
	QMainWindow::closeEvent(event);
}

void MainWindow::editEngineSettings()
{
	QString settingsFilename = mSettings.value(getSettingsFilenameKey()).toString();

	SettingsEditor editor(settingsFilename, mEngineRoot->engineSettings, this);
	auto dialog = createDialogModal(&editor, "Settings");
	dialog->setMinimumWidth(500);
	dialog->setMinimumHeight(500);

	if (dialog->exec() == QDialog::Accepted)
	{
		settingsFilename = editor.getSettingsFilename();
		mSettings.setValue(getSettingsFilenameKey(), settingsFilename);

		mEngineRoot->engineSettings = editor.getJson();
		writeJsonFile(editor.getJson(), settingsFilename.toStdString());
	}
}

void MainWindow::addToolWindow(const QString& windowName, QWidget* window)
{
	window->setWindowTitle(windowName);
	window->setObjectName(windowName);

	QAction* action = new QAction(windowName, ui->menuView);
	ui->menuView->insertAction(mViewMenuToolWindowSeparator, action);
	
	action->setData((int)mToolActions.size());
	action->setCheckable(true);
	action->setChecked(true);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(toolWindowActionToggled(bool)));
	mToolActions.push_back(action);

	mToolWindowManager->addToolWindow(window, ToolWindowManager::LastUsedArea);
}

void MainWindow::raiseToolWindow(QWidget* widget)
{
	ToolWindowManagerArea* area = mToolWindowManager->areaOf(widget);
	if (area)
	{
		area->activateWindow();
	}
}

QMenuBar* MainWindow::getMenuBar() const
{
	return ui->menuBar;
}

void MainWindow::toolWindowActionToggled(bool state)
{
	int index = static_cast<QAction*>(sender())->data().toInt();
	QWidget *toolWindow = mToolWindowManager->toolWindows()[index];
	// TODO: restore tool to previous location
	mToolWindowManager->moveToolWindow(toolWindow, state ? ToolWindowManager::LastUsedArea : ToolWindowManager::NoArea);
}

void MainWindow::toolWindowVisibilityChanged(QWidget* toolWindow, bool visible)
{
	int index = mToolWindowManager->toolWindows().indexOf(toolWindow);
	mToolActions[index]->blockSignals(true);
	mToolActions[index]->setChecked(visible);
	mToolActions[index]->blockSignals(false);
}

void MainWindow::setProjectFilename(const QString& filename)
{
	if (!mProjectFilename.isEmpty())
	{
		std::string folder = std::filesystem::path(mProjectFilename.toStdString()).parent_path().string();
		auto& pathList = osgDB::Registry::instance()->getDataFilePathList();
		if (auto i = std::find(pathList.begin(), pathList.end(), folder); i != pathList.end())
		{
			pathList.erase(i);
		}
	}

	mProjectFilename = filename;
	QString title = QApplication::applicationName();
	if (!mProjectFilename.isEmpty())
	{
		title += " - " + mProjectFilename;
	}
	setWindowTitle(title);

	std::string folder = std::filesystem::path(mProjectFilename.toStdString()).parent_path().string();
	osgDB::Registry::instance()->getDataFilePathList().push_back(folder);
}
