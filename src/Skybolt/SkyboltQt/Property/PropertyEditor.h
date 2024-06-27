/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "DefaultEditorWidgets.h"
#include "SkyboltQt/Property/PropertyModel.h"
#include "SkyboltQt/SkyboltQtFwd.h"

#include <QWidget>
#include <memory>
#include <typeindex>

class QGridLayout;

class PropertyEditor : public QWidget
{
	Q_OBJECT
public:
	PropertyEditor(const PropertyEditorWidgetFactoryMap& factoryMap = getDefaultEditorWidgetFactoryMap(), QWidget* parent = nullptr);

	void setModel(const PropertiesModelPtr& model);
	PropertiesModelPtr getModel() const { return mModel; }

private slots:
	void modelReset(PropertiesModel* model);

private:
	QWidget* createEditor(QtProperty* property);

private:
	PropertiesModelPtr mModel;
	QGridLayout* mGridLayout;
	PropertyEditorWidgetFactoryMap mFactoryMap;
};
