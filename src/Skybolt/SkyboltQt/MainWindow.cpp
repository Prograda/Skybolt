/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Engine/EngineSettingsSerialization.h"
#include "QtUtil/QtDialogUtil.h"
#include "QtUtil/QtTimerUtil.h"
#include "QtUtil/RecentFilesMenuPopulator.h"
#include "Property/PropertyEditor.h"
#include "Scenario/ScenarioWorkspace.h"
#include "Widgets/SettingsEditor.h"

#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/Json/WriteJsonFile.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/EngineStats.h>

#include <filesystem>

#include <ToolWindowManager/ToolWindowManagerArea.h>

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>

#include <algorithm>
#include <chrono>
#include <sstream>

using namespace skybolt;
using namespace skybolt::sim;

MainWindow::MainWindow(const MainWindowConfig& windowConfig) :
	QMainWindow(windowConfig.parent, windowConfig.flags),
	ui(new Ui::MainWindow),
	mWorkspace(windowConfig.workspace),
	mEngineRoot(windowConfig.engineRoot),
	mSettings(QApplication::applicationName())
{
	assert(mWorkspace);
	assert(mEngineRoot);

	connectJsonScenarioSerializable(*mWorkspace, this);
	connect(mWorkspace.get(), &ScenarioWorkspace::scenarioFilenameChanged, this, &MainWindow::scenarioFilenameChanged);
	scenarioFilenameChanged(mWorkspace->getScenarioFilename());

	RecentFilesMenuPopulator::FileOpener fileOpener = [this](const QString& filename) {
		openScenario(filename, OverwriteMode::PromptToSaveChanges);
	};

	ui->setupUi(this);
	mViewMenuToolWindowSeparator = ui->menuView->addSeparator();
	mRecentFilesMenuPopulator.reset(new RecentFilesMenuPopulator(ui->menuRecentFiles, &mSettings, fileOpener));

	// Connect menus
	QObject::connect(ui->actionNewScenario, SIGNAL(triggered()), this, SLOT(newScenario()));
	QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openScenario()));
	QObject::connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveScenario()));
	QObject::connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveScenarioAs()));
	QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(exit()));
	QObject::connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(editEngineSettings()));

	mToolWindowManager = new ToolWindowManager(this);
	// setup tool window manager
	connect(mToolWindowManager, SIGNAL(toolWindowVisibilityChanged(QWidget*, bool)), this, SLOT(toolWindowVisibilityChanged(QWidget*, bool)));

	setCentralWidget(mToolWindowManager);

	setStatusBar(new QStatusBar(this));

	createAndStartIntervalTimer(100, this, [this] { update(); });
}

MainWindow::~MainWindow() = default;

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

static QVariantMap toVariantMap(const QByteArray& array)
{
	QVariantMap variantMap;
	QDataStream stream(const_cast<QByteArray*>(&array), QIODevice::OpenMode::enum_type::ReadOnly);
	stream >> variantMap;
	return variantMap;
}

void MainWindow::restoreToolWindowsState(const QString& stateBase64)
{
	mToolWindowManager->restoreState(toVariantMap(QByteArray::fromBase64(stateBase64.toUtf8())));
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
			return saveScenario();
		case QMessageBox::Discard:
			return true;
		case QMessageBox::Cancel:
			return false;
		default:
			assert(!"Should never get here");
	}
	return true;
}

void MainWindow::newScenario()
{
	newScenario(OverwriteMode::PromptToSaveChanges);
}

void MainWindow::newScenario(OverwriteMode mode)
{
	if (mode == OverwriteMode::PromptToSaveChanges && !saveChangesAndContinue())
	{
		return;
	}

	mWorkspace->newScenario();
}

QString scenarioFileExtension = ".scn";
QString scenarioFileFilter = "Scenario Files (*" + scenarioFileExtension + ")";

QString defaultDirKey = "defaultDir";

QString MainWindow::getDefaultScenarioDirectory() const
{
	QString defaultDir = mSettings.value(defaultDirKey).toString();
	if (defaultDir.isEmpty())
	{
		defaultDir = QDir::homePath();
	}
	return defaultDir;
}

void MainWindow::openScenario()
{
	if (!saveChangesAndContinue())
	{
		return;
	}

	QString filename = QFileDialog::getOpenFileName(nullptr, tr("Open Project"),
		getDefaultScenarioDirectory(), scenarioFileFilter);

	mRecentFilesMenuPopulator->addFilename(filename);
	if (!filename.isEmpty())
	{
		OverwriteMode mode = OverwriteMode::OverwriteWithoutPrompt; // we've already prompted the user, so don't do it again
		openScenario(filename, mode);
	}
}

void MainWindow::openScenario(const QString& filename, OverwriteMode mode)
{
	if (mode == OverwriteMode::PromptToSaveChanges && !saveChangesAndContinue())
	{
		return;
	}

	QDir currentDir;
	mSettings.setValue(defaultDirKey, currentDir.absoluteFilePath(filename));

	if (auto errorMessage = mWorkspace->loadScenario(filename); errorMessage)
	{
		QMessageBox::critical(this, "", *errorMessage);
	}
}

static QByteArray toByteArray(const QVariantMap& variant)
{
	QByteArray array;
	QDataStream stream(&array, QIODevice::OpenMode::enum_type::WriteOnly);
	stream << variant;
	return array;
}

void MainWindow::readScenario(const nlohmann::json& json)
{
	ifChildExists(json, "mainWindowGeometry", [this] (const nlohmann::json& child) {
		QString str = QString::fromStdString(child.get<std::string>());
		restoreGeometry(QByteArray::fromBase64(str.toUtf8()));
	});

	ifChildExists(json, "windowLayout", [this] (const nlohmann::json& child) {
		QString str = QString::fromStdString(child.get<std::string>());
		mToolWindowManager->restoreState(toVariantMap(QByteArray::fromBase64(str.toUtf8())));
	});
}

void MainWindow::writeScenario(nlohmann::json& json) const
{
	json["windowLayout"] = toByteArray(mToolWindowManager->saveState()).toBase64().toStdString();
	json["mainWindowGeometry"] = saveGeometry().toBase64().toStdString();
}

bool MainWindow::saveScenario()
{
	if (mWorkspace->getScenarioFilename().isEmpty())
	{
		saveScenarioAs();
	}
	else
	{
		if (auto errorMessage = mWorkspace->saveScenario(mWorkspace->getScenarioFilename()); errorMessage)
		{
			QMessageBox::critical(this, "", *errorMessage);
		}
	}
	return false;
}

bool MainWindow::saveScenarioAs()
{
	QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save Scenario"),
		getDefaultScenarioDirectory(), scenarioFileFilter);

	if (!fileName.isEmpty())
	{
		if (!fileName.endsWith(scenarioFileExtension, Qt::CaseInsensitive))
			fileName += scenarioFileExtension;

		if (auto errorMessage = mWorkspace->saveScenario(fileName); errorMessage)
		{
			QMessageBox::critical(this, "", *errorMessage);
		}

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

void MainWindow::scenarioFilenameChanged(const QString& filename)
{
	QString title = QApplication::applicationName();
	if (!filename.isEmpty())
	{
		title += " - " + filename;
	}
	setWindowTitle(title);
}
