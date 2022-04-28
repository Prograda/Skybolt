/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/Registry.h"
#include <SkyboltCommon/Range.h>
#include <boost/signals2.hpp>
#include <string>

struct DataSeries
{
	typedef std::vector<double> DoubleVector;
	std::map<std::string, DoubleVector> data;

	boost::signals2::signal<void(skybolt::IntRangeInclusive)> valuesAdded;
	boost::signals2::signal<void(skybolt::IntRangeInclusive)> valuesChanged;
	boost::signals2::signal<void(skybolt::IntRangeInclusive)> valuesRemoved;
	boost::signals2::signal<void(const std::string&)> keyAdded;
	boost::signals2::signal<void(const std::string&)> keyRemoved;
};

struct NamedDataSeries
{
	DataSeriesPtr data;
	std::string name;

	const std::string& getName() const { return name; }
};

typedef Registry<NamedDataSeries> DataSeriesRegistry;