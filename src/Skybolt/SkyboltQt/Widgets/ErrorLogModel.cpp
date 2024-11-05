/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ErrorLogModel.h"

#include <assert.h>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>

namespace bl = boost::log;

class LabelLogSink : public bl::sinks::basic_formatted_sink_backend<char, bl::sinks::synchronized_feeding>
{
public:
	LabelLogSink(std::function<void(bl::trivial::severity_level level, const QString&)> fn) :
		mFn(fn)
	{
	}

    void consume(const bl::record_view& rec, const std::string& str) {
		if (auto severityAttribute = rec["Severity"]; severityAttribute)
		{
			if (auto level = severityAttribute.extract<boost::log::trivial::severity_level>(); level)
			{
				mFn(*level, QString::fromStdString(str));
			}
		}
    }

private:
	std::function<void(bl::trivial::severity_level level, const QString&)> mFn;
};

ErrorLogModel::ErrorLogModel(QObject* parent) :
	QObject(parent)
{
}

void ErrorLogModel::append(const Item& item)
{
	mItems.push_back(item);
	Q_EMIT itemAppended(item);
}

void ErrorLogModel::clear()
{
	mItems.clear();
	Q_EMIT cleared();
}

static ErrorLogModel::Severity toErrorLogModelSeverity(bl::trivial::severity_level level)
{
	switch (level)
	{
	case bl::trivial::severity_level::warning:
		return ErrorLogModel::Severity::Warning;
	}
	return ErrorLogModel::Severity::Error;
}

void connectToBoostLogger(QPointer<ErrorLogModel> model)
{
	auto sink = boost::make_shared<LabelLogSink>([model = std::move(model)] (bl::trivial::severity_level level, const QString& message) {
		if (model)
		{
			ErrorLogModel::Item item;
			item.dateTime = QDateTime::currentDateTime();
			item.severity = toErrorLogModelSeverity(level);
			item.message = message;
			model->append(item);
		}
	});

	using sink_t = bl::sinks::synchronous_sink<LabelLogSink>;
	auto sinkWrapper = boost::make_shared<sink_t>(sink);
	sinkWrapper->set_filter(bl::trivial::severity >= bl::trivial::warning);
    bl::core::get()->add_sink(sinkWrapper);
}
