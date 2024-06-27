/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/Property/PropertyModel.h"
#include <SkyboltReflection/Reflection.h>
#include <SkyboltCommon/Updatable.h>

#include <functional>
#include <optional>

struct QtPropertyUpdaterApplier
{
	QtPropertyPtr property; //!< Never null
	PropertiesModel::QtPropertyUpdater updater; //!< Never null
	PropertiesModel::QtPropertyApplier applier; //!< Null for read-only properties
};

using ReflInstanceGetter = std::function<std::optional<skybolt::refl::Instance>()>;

std::optional<QtPropertyUpdaterApplier> reflPropertyToQt(skybolt::refl::TypeRegistry& typeRegistry, const ReflInstanceGetter& instanceGetter, const skybolt::refl::PropertyPtr& property);

void addRttrPropertiesToModel(skybolt::refl::TypeRegistry& typeRegistry, PropertiesModel& model, const std::vector<skybolt::refl::PropertyPtr>& properties, const ReflInstanceGetter& instanceGetter);