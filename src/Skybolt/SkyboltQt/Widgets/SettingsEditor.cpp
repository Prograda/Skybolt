/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SettingsEditor.h"
#include "Property/PropertyEditor.h"
#include "QtUtil/QtScrollAreaUtil.h"

#include <QBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <boost/algorithm/string.hpp>  
#include <memory>

static QVariant toQVariant(const nlohmann::json& j)
{
	auto type = j.type();
	switch (type)
	{
	case nlohmann::json::value_t::boolean:
		return j.get<bool>();
	case nlohmann::json::value_t::string:
		return QString::fromStdString(j.get<std::string>());
	case nlohmann::json::value_t::number_integer:
	case nlohmann::json::value_t::number_unsigned:
		return j.get<int>();
	case nlohmann::json::value_t::number_float:
		return j.get<double>();
	default:
		return QVariant();
	}
}

static void setJsonQVariant(nlohmann::json& j, const std::string& key, const QVariant& variant)
{
	assert(!key.empty());

	// Convert value to json
	nlohmann::json value;
	switch (variant.type())
	{
	case QVariant::String:
		value = variant.toString().toStdString();
		break;
	case QVariant::Bool:
		value = variant.toBool();
		break;
	case QVariant::Int:
		value = variant.toInt();
		break;
	case QVariant::Double:
		value = variant.toDouble();
		break;
	default:
		assert(!"QVariant type is not supported by json");
	}

	// Write property with hierarchy
	std::vector<std::string> keyParts;
	boost::split(keyParts, key, boost::is_any_of("."), boost::token_compress_on);

	nlohmann::json* object = &j;
	for (int i = 0; i < int(keyParts.size()-1); ++i)
	{
		object = &((*object)[keyParts[i]]);
	}
	(*object)[keyParts.back()] = value;
}

static void createPropertiesRecursive(std::vector<QtPropertyPtr>& properties, const nlohmann::json& j, const std::string& propertyNamePrefix = "")
{
	for (const auto& item : j.items())
	{
		if (item.value().is_object())
		{
			createPropertiesRecursive(properties, item.value(), item.key() + ".");
		}

		QVariant variant = toQVariant(item.value());
		if (!variant.isNull())
		{
			if (item.key().find(".") != std::string::npos)
			{
				throw std::runtime_error("Settings property name must not contain period (.)");
			}

			auto property = createQtProperty(QString::fromStdString(propertyNamePrefix + item.key()), variant);
			properties.push_back(property);
		}
	}
}

SettingsEditor::SettingsEditor(const QString& settingsFilename, const nlohmann::json& settings, QWidget* parent)
{
	std::vector<QtPropertyPtr> properties;
	createPropertiesRecursive(properties, settings);

	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("Note: some settings will not take effect until application restart.", this));

	{
		mSettingsFilenameProperty = createQtProperty("Settings filename", settingsFilename);
		PropertyEditor* editor = new PropertyEditor();
		editor->setModel(std::make_shared<PropertiesModel>(std::vector<QtPropertyPtr>{ mSettingsFilenameProperty }));
		layout->addWidget(editor, 0);
	}
	{
		mSettingsModel = std::make_shared<PropertiesModel>(properties);
		PropertyEditor* editor = new PropertyEditor();
		editor->setModel(mSettingsModel);
		layout->addWidget(wrapWithVerticalScrollBar(editor, this), 1);
	}
}

QString SettingsEditor::getSettingsFilename() const
{
	return mSettingsFilenameProperty->value.toString();
}

nlohmann::json SettingsEditor::getJson() const
{
	nlohmann::json j;
	for (const auto& item : mSettingsModel->getProperties())
	{
		setJsonQVariant(j, item->name.toStdString(), item->value);
	}

	return j;
}