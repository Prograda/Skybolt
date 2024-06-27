/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <SkyboltQt/QtUtil/QtDateTimeUtil.h>

#include <QDateTime>
#include <QTimeZone>

TEST_CASE("Convert QDateTime to/from julian date")
{
	// Tested against https://www.aavso.org/jd-calculator
	int year = 2017;
	int month = 8;
	int day = 7;
	int hour = 10;
	int minute = 59;
	int second = 0;

	QDateTime dateTime(QDate(year, month, day), QTime(hour, minute, second), QTimeZone::utc());
	CHECK(2457972.95764 == Approx(qdateTimeToJulianDate(dateTime)).margin(0.00001));
	CHECK(dateTime == julianDateToQDateTime(2457972.95764));
}
