/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgLogHandler.h"
#include <SkyboltCommon/Logging/Logging.h>
#include <assert.h>
#include <osg/Notify>


namespace skybolt {
namespace vis {

class OsgLogHandler : public osg::NotifyHandler
{
	void notify(osg::NotifySeverity osgSeverity, const char *message) override
	{
		switch (osgSeverity)
		{
			case osg::NotifySeverity::DEBUG_FP:
			case osg::NotifySeverity::DEBUG_INFO:
				SKYBOLT_LOG(debug) << message; break;
			case osg::NotifySeverity::NOTICE:
			case osg::NotifySeverity::INFO:
			case osg::NotifySeverity::ALWAYS:
				SKYBOLT_LOG(info) << message; break;
			case osg::NotifySeverity::WARN:
				SKYBOLT_LOG(error) << message; break; // treat OSG 'warnings' as errors because OSG reports shader compilation errors as warnings
			case osg::NotifySeverity::FATAL:
				SKYBOLT_LOG(fatal) << message; break;
			default:
				assert(!"Not implented");
				SKYBOLT_LOG(info) << message; break;
		}
		
	}
};

void forwardOsgLogToBoost()
{
	osg::setNotifyHandler(new OsgLogHandler());
}

} // namespace vis
} // namespace skybolt
