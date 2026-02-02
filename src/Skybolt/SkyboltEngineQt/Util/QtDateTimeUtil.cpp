#include "QtDateTimeUtil.h"

#include <SkyboltSim/Physics/Astronomy.h>

#include <QDate>
#include <QTimeZone>

QDateTime julianDateToQDateTime(double julianDate)
{
	int year, month, day;
	skybolt::sim::julianDateToYmd(julianDate, year, month, day);
	QDate date(year, month, day);

	int hour, minute;
	double second;
	skybolt::sim::julianDateToHms(julianDate, hour, minute, second);
	QTime time(hour, minute, (int)second);

	return QDateTime(date, time, QTimeZone::utc());
}

double qdateTimeToJulianDate(const QDateTime& dateTime)
{
	const QDate& date = dateTime.date();
	const QTime& time = dateTime.time();
	double hourF = double(time.hour()) + double(time.minute()) / 60.0 + double(time.second()) / (60.0*60.0);
	return skybolt::sim::calcJulianDate(date.year(), date.month(), date.day(), hourF);
}