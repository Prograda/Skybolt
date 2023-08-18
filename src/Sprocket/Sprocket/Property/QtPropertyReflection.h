/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/Property/PropertyModel.h"
#include <SkyboltSim/Reflection.h>
#include <SkyboltCommon/Updatable.h>

#include <functional>
#include <optional>

struct QtPropertyUpdaterApplier
{
	QtPropertyPtr property; //!< Never null
	PropertiesModel::QtPropertyUpdater updater; //!< Never null
	PropertiesModel::QtPropertyApplier applier; //!< Null for read-only properties
};

using RttrInstanceGetter = std::function<rttr::instance()>;

std::optional<QtPropertyUpdaterApplier> rttrPropertyToQt(const RttrInstanceGetter& instanceGetter, const rttr::property& property);

void addRttrPropertiesToModel(PropertiesModel& model, const rttr::array_range<rttr::property>& properties, const RttrInstanceGetter& instanceGetter);