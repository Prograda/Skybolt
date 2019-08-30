/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Functions/FlowFunctionRegistry.h"
#include <Sprocket/SprocketFwd.h>
#include <Sprocket/DataSeries/DataSeries.h>
#include <SkyboltEngine/TimeSource.h>
#include <SkyboltEngine/EntityFactory.h>
#include <SkyboltEngine/Input/InputPlatform.h>
#include <SkyboltCommon/File/FileLocator.h>

class QwtPlot;

struct NodeContext
{
	QwtPlot* plot;
	bool chartSeriesChanged = false;
	skybolt::EntityFactory* entityFactory;
	skybolt::sim::World* simWorld;
	skybolt::InputPlatform* inputPlatform;
	skybolt::TimeSource* timeSource;
	const skybolt::sim::NamedObjectRegistry* namedObjectRegistry;
	FlowFunctionRegistry* flowFunctionRegistry;
	std::shared_ptr<DataSeriesRegistry> dataSeriesRegistry;
	skybolt::file::FileLocator fileLocator;
};
