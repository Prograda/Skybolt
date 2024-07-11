/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PropertyModel.h"

#include <boost/scope_exit.hpp>
#include <assert.h>

QtPropertyPtr createQtProperty(const QString& name, const QVariant& value)
{
	auto property = std::make_shared<QtProperty>();
	property->name = name;
	property->value = value;
	return property;
}

void PropertiesModel::update()
{
	mCurrentlyUpdating = true;
	BOOST_SCOPE_EXIT(&mCurrentlyUpdating) {
        mCurrentlyUpdating = false;
    } BOOST_SCOPE_EXIT_END

	for (const auto& entry : mPropertyUpdaters)
	{
		entry.second(*entry.first);
	}
}

void PropertiesModel::addProperty(const QtPropertyPtr& property, QtPropertyUpdater updater, QtPropertyApplier applier)
{
	mProperties.push_back(property);
	if (updater)
	{
		mPropertyUpdaters[property] = updater;
	}

	if (applier)
	{
		connect(property.get(), &QtProperty::valueChanged, this, [=]() {
			if (!mCurrentlyUpdating)
			{
				applier(*property);
			}
		});
	}
}

PropertiesModel::PropertiesModel()
{
}

PropertiesModel::PropertiesModel(const std::vector<QtPropertyPtr>& properties) :
	mProperties(properties)
{
}
