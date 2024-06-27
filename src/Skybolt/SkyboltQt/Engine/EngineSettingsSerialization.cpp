#include "EngineSettingsSerialization.h"

#include <SkyboltCommon/File/OsDirectories.h>
#include <SkyboltCommon/Json/ReadJsonFile.h>
#include <SkyboltCommon/Json/WriteJsonFile.h>
#include <SkyboltEngine/EngineSettings.h>

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include <filesystem>

using namespace skybolt;

const QString& getSettingsFilenameKey()
{
	static QString settingsFilenameKey = "settingsFilename";
	return settingsFilenameKey;
}

// Read the engine settings file if it already exists, otherwise prompts user to create one.
// The engine settings file is different to the QSettings, the former configures the engine
// while the latter configures UI defaults.
nlohmann::json readOrCreateEngineSettingsFile(QSettings& settings, QWidget* parent)
{
	nlohmann::json result = createDefaultEngineSettings();
	QString settingsFilename = settings.value(getSettingsFilenameKey()).toString();

	while (settingsFilename.isEmpty() || !QFile(settingsFilename).exists())
	{
		QMessageBox box;
		box.setWindowTitle("Settings");
		box.setText("Settings file required to run.\nClick Create to create a new file, or Load to load an existing file");
		QAbstractButton* createButton = box.addButton("Create...", QMessageBox::YesRole);
		QAbstractButton* loadButton = box.addButton("Load...", QMessageBox::YesRole);

		box.exec();
		if (box.clickedButton() == createButton)
		{
			auto appUserDataDir = file::getAppUserDataDirectory("Skybolt");
			settingsFilename = QString::fromStdString(appUserDataDir.append("Settings.json").string());

			settingsFilename = QFileDialog::getSaveFileName(parent, "Create Settings File", settingsFilename, "Json (*.json)");

			if (!settingsFilename.isEmpty())
			{
				// Create file
				settings.setValue(getSettingsFilenameKey(), settingsFilename);
				std::filesystem::create_directories(file::Path(settingsFilename.toStdString()).parent_path());
				writeJsonFile(result, settingsFilename.toStdString());
				return result;
			}
		}
		else if (box.clickedButton() == loadButton)
		{
			settingsFilename = QFileDialog::getOpenFileName(parent, "Load Settings File", settingsFilename, "Json (*.json)");
		 	settings.setValue(getSettingsFilenameKey(), settingsFilename);

			if (!settingsFilename.isEmpty())
			{
				break;
			}
		}
		else
		{
			throw std::runtime_error("Settings file not specified. Program will now exit.");
		}
	}

	// Load file
	result.update(readJsonFile(settingsFilename.toStdString()), /* merge_objects */ true);
	return result;
}