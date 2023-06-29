/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/Property/PropertyModel.h"
#include "Sprocket/SprocketFwd.h"

#include <QWidget>
#include <memory>
#include <typeindex>

class QGridLayout;

using PropertyEditorWidgetFactory = std::function<QWidget*(QtProperty& property)>;
using PropertyEditorWidgetFactoryMap = std::map<std::type_index, PropertyEditorWidgetFactory>;

class PropertyEditor : public QWidget
{
	Q_OBJECT
public:
	PropertyEditor(const PropertyEditorWidgetFactoryMap& factoryMap, QWidget* parent = nullptr);

	void setModel(const PropertiesModelPtr& model);

private slots:
	void modelReset(PropertiesModel* model);

private:
	QWidget* createEditorInEnabledState(QtProperty& property);
	QWidget* createEditor(QtProperty& property);

private:
	PropertiesModelPtr mModel;
	QGridLayout* mGridLayout;
	PropertyEditorWidgetFactoryMap mFactoryMap;
};
