/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PythonComponent.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Physics/Astronomy.h>

#include <filesystem>

#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/chrono.h>
#pragma pop_macro("slots")
#include "datetime.h"
#include "object.h"

#include <assert.h>
#include <fstream>
#include <istream>
#include <sstream>

namespace py = pybind11;

namespace skybolt {

struct PythonData
{
	py::object object;
};

static py::handle julianDateToPythonDateTime(double julianDate)
{
	int year, month, day, hour, minute;
	double second;
	sim::julianDateToYmd(julianDate, year, month, day);
	sim::julianDateToHms(julianDate, hour, minute, second);

	// Lazy initialise the PyDateTime import
	if (!PyDateTimeAPI) { PyDateTime_IMPORT; }

	py::handle time = PyDateTimeAPI->DateTime_FromDateAndTime(
		year,
		month,
		day,
		hour,
		minute,
		int(second),
		int(1000000.0 * double(second - (int)second)),
		PyDateTime_TimeZone_UTC, PyDateTimeAPI->DateTimeType);

	return time;
}

PythonComponent::PythonComponent(sim::Entity* entity, JulianDateProvider julianDateProvider, const std::string& moduleName, const std::string& className) :
	mEntity(entity),
	mPythonData(new PythonData),
	mJulianDateProvider(julianDateProvider)
{
	assert(mEntity);
	auto utcTime = julianDateToPythonDateTime(mJulianDateProvider());
	mPythonData->object = py::module::import(moduleName.c_str()).attr(className.c_str())(mEntity, utcTime);
}

PythonComponent::~PythonComponent()
{
}

void PythonComponent::updatePreDynamics(sim::TimeReal dt, sim::TimeReal dtWallClock)
{
	auto utcTime = julianDateToPythonDateTime(mJulianDateProvider());
	mPythonData->object.attr("update")(utcTime);
}

} // namespace skybolt