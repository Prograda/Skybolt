/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/SimMath.h>
#include <QStringList>
#include <QVector3D>
#include <string>
#include <vector>

inline skybolt::sim::Vector3 toVector3(const QVector3D& v)
{
	return skybolt::sim::Vector3(v.x(), v.y(), v.z());
}

inline QVector3D toQVector3D(const skybolt::sim::Vector3& v)
{
	return QVector3D(v.x, v.y, v.z);
}

inline QStringList toQStringList(const std::vector<std::string>& strs)
{
	QStringList r;
	r.reserve((int)strs.size());
	for (const std::string& str : strs)
	{
		r.push_back(QString::fromStdString(str));
	}
	return r;
}
