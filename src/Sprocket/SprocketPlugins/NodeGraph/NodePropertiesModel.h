/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Nodes/SimpleNdm.h"
#include <Sprocket/PropertyEditor.h>
#include <QLabel>
#include <map>

class NodePropertiesModel : public PropertiesModel
{
	Q_OBJECT
public:
	NodePropertiesModel(SimpleNdm* node);

private:
	SimpleNdm * mNode;
	std::map<QtPropertyPtr, std::shared_ptr<VariantNodeData>> propertyNodeDataMap;
	std::map<int, QtPropertyPtr> mIndexToPropertyMap;
};
