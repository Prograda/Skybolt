/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltQtFwd.h>
#include <nlohmann/json.hpp>
#include <QWidget>

class SettingsEditor : public QWidget
{
public:
	SettingsEditor(const QString& settingsFilename, const nlohmann::json& settings, QWidget* parent = nullptr);

	QString getSettingsFilename() const;
	nlohmann::json getJson() const;

private:
	std::shared_ptr<struct QtProperty> mSettingsFilenameProperty;
	PropertiesModelPtr mSettingsModel;
};