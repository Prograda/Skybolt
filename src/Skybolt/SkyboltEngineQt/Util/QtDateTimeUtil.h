#pragma once

class QDateTime;

QDateTime julianDateToQDateTime(double julianDate);
double qdateTimeToJulianDate(const QDateTime& dateTime);