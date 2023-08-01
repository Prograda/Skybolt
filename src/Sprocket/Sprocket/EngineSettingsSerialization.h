#pragma once

#include <QSettings>
#include <nlohmann/json.hpp>

const QString& getSettingsFilenameKey();

nlohmann::json readOrCreateEngineSettingsFile(QSettings& settings, QWidget* parent = nullptr);
