/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ConsoleSink.h"

#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>

namespace bl = boost::log;

namespace skybolt {

void addConsoleLogSink()
{
	using console_sink = bl::sinks::synchronous_sink<bl::sinks::text_ostream_backend>;
    boost::shared_ptr<console_sink> consoleSink = boost::make_shared<console_sink>();
    consoleSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));

	bl::core::get()->add_sink(consoleSink);
}

} // namespace skybolt