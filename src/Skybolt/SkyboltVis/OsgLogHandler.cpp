/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgLogHandler.h"
#include <osg/Notify>

#include <boost/log/trivial.hpp>

namespace skybolt {
namespace vis {

// From https://stackoverflow.com/questions/43734971/c-pass-boostlog-severity-level-as-argument-to-function
#define LOG_TRIVIAL(lvl)\
    BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(),\
        (::boost::log::keywords::severity = lvl))

class OsgLogHandler : public osg::NotifyHandler
{
	void notify(osg::NotifySeverity osgSeverity, const char *message) override
	{
		auto boostSeverity = toBoostLogSeverity(osgSeverity);
		LOG_TRIVIAL(boostSeverity) << message;
	}

	static boost::log::trivial::severity_level toBoostLogSeverity(osg::NotifySeverity severity)
	{
		using boost_severity_level = boost::log::trivial::severity_level;
		switch (severity)
		{
			case osg::NotifySeverity::DEBUG_FP:
			case osg::NotifySeverity::DEBUG_INFO:
				return boost_severity_level::debug;
			case osg::NotifySeverity::NOTICE:
			case osg::NotifySeverity::INFO:
			case osg::NotifySeverity::ALWAYS:
				return boost_severity_level::info;
			case osg::NotifySeverity::WARN:
				return boost_severity_level::error; // treat OSG 'warnings' as errors because OSG reports shader compilation errors as warnings
			case osg::NotifySeverity::FATAL:
				return boost_severity_level::fatal;
			default:
				assert(!"Not implented");
				return boost_severity_level::info;
		}
	}
};

void forwardOsgLogToBoost()
{
	osg::setNotifyHandler(new OsgLogHandler());
}

} // namespace vis
} // namespace skybolt
