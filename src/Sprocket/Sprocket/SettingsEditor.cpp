/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SettingsEditor.h"
#include "PropertyEditor.h"

#include <QBoxLayout>
#include <QScrollArea>
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
	switch (variant.type())
	{
	case QVariant::String:
		j[key] = variant.toString().toStdString();
		break;
	case QVariant::Bool:
		j[key] = variant.toBool();
		break;
	case QVariant::Int:
		j[key] = variant.toInt();
		break;
	case QVariant::Double:
		j[key] = variant.toDouble();
		break;
	default:
		assert(!"QVariant type is not supported by json");
	}
}

SettingsEditor::SettingsEditor(const QString& settingsFilename, const nlohmann::json& settings, QWidget* parent)
{
	std::vector<QtPropertyPtr> properties;
	for (const auto& item : settings.items())
	{
		QVariant variant = toQVariant(item.value());
		if (!variant.isNull())
		{
			auto property = PropertiesModel::createVariantProperty(QString::fromStdString(item.key()), variant);
			properties.push_back(property);
		}
	}

	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);

	{
		mSettingsFilenameProperty = PropertiesModel::createVariantProperty("Settings filename", settingsFilename);
		PropertyEditor* editor = new PropertyEditor({});
		editor->setModel(std::make_shared<PropertiesModel>(std::vector<QtPropertyPtr>{ mSettingsFilenameProperty }));
		layout->addWidget(editor, 0);
	}
	{
		mSettingsModel = std::make_shared<PropertiesModel>(properties);
		PropertyEditor* editor = new PropertyEditor({});
		editor->setModel(mSettingsModel);

		QScrollArea* scrollArea = new QScrollArea;
		scrollArea->setWidget(editor);
		scrollArea->setWidgetResizable(true);
		layout->addWidget(scrollArea, 1);
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
		if (auto variantProperty = dynamic_cast<VariantProperty*>(item.get()))
		{
			setJsonQVariant(j, item->name.toStdString(), variantProperty->value);
		}
	}

	return j;
}