/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QString>

class ErrorLogModel : public QObject
{
	Q_OBJECT
public:
	ErrorLogModel(QObject* parent = nullptr);

	enum class Severity
	{
		Warning,
		Error
	};

	struct Item
	{
		QDateTime dateTime;
		Severity severity;
		QString message;
	};

	void append(const Item& item);
	void clear();

	const std::vector<Item>& getItems() const { return mItems; }


	Q_SIGNAL void itemAppended(const Item& item);
	Q_SIGNAL void cleared();

private:
	std::vector<Item> mItems;
};

void connectToBoostLogger(QPointer<ErrorLogModel> model);
