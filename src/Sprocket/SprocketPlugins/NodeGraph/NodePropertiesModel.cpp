/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "NodePropertiesModel.h"

NodePropertiesModel::NodePropertiesModel(SimpleNdm* node) :
	mNode(node)
{
	SimpleNdm::NodeDataVector* params = node->getGuiParams();
	int i = 0;
	for (std::shared_ptr<QtNodes::NodeData>& param : *params)
	{
		QtPropertyPtr property = nullptr;
		std::shared_ptr<VariantNodeData> variantNodeData = std::dynamic_pointer_cast<VariantNodeData>(param);
		if (variantNodeData)
		{
			if (EnumNodeData* nodeData = dynamic_cast<EnumNodeData*>(param.get()))
			{
				QStringList stringList;
				for (const std::string& str : *nodeData->enumStrings)
				{
					stringList.push_back(QString::fromStdString(str));
				}

				auto enumProperty = createEnumProperty(param->type().name, stringList);
				enumProperty->setValue(nodeData->getValueAsInt());
				property = enumProperty;
			}
			else
			{
				QVariant value = variantNodeData->toVariant();
				if (value.type() != QVariant::Invalid)
				{
					auto variantProperty = createVariantProperty(param->type().name, value);
					variantProperty->setValue(value);
					property = variantProperty;
				}
			}
		}

		if (property)
		{
			property->setEnabled(node->getInputParams()->at(i) == nullptr); // enabled if input is not connect
			mProperties.push_back(property);
			propertyNodeDataMap[property] = variantNodeData;
			mIndexToPropertyMap[i] = property;

			connect(property.get(), &QtProperty::valueChanged, this, [this, variantNodeData, property]() {
				QVariant variant;
				if (auto variantProperty = dynamic_cast<VariantProperty*>(property.get()))
				{
					variant = variantProperty->value;
				}
				else if (auto enumProperty = dynamic_cast<EnumProperty*>(property.get()))
				{
					variant = enumProperty->value;
				}
				else
				{
					return;
				}
				assert(variantNodeData);
				variantNodeData->fromVariant(variant);
				mNode->eval(); // TODO: Should only be setting the input as dirty rather than doing full evaluation. Consider changing graph to be pull instead of push.
			});
		}

		++i;
	}

	QObject::connect(mNode, &SimpleNdm::inputChanged, this, [&](int index)
	{
		auto it = mIndexToPropertyMap.find(index);
		if (it != mIndexToPropertyMap.end())
		{
			it->second->setEnabled(mNode->getInputParams()->at(index) == nullptr);
		}
	});
}
