/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisRoot.h"
#include <SkyboltCommon/Logging/Logging.h>
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgLogHandler.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderOperation/RenderTarget.h"
#include "SkyboltVis/Window/Window.h"
#include <SkyboltCommon/VectorUtility.h>

#include <osgViewer/CompositeViewer>
#include <format>

#ifdef OSG_LIBRARY_STATIC
#include <osgDB/Registry>
// include the plugins we need
USE_OSGPLUGIN(bmp)
#ifdef BUILD_WITH_OSG_CURL_PLUGIN
	USE_OSGPLUGIN(curl)
#endif
USE_OSGPLUGIN(dds)
USE_OSGPLUGIN(freetype)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(tga)

USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

// include the platform specific GraphicsWindow implementation
USE_GRAPHICSWINDOW()
#endif

namespace skybolt {
namespace vis {

VisRoot::VisRoot(const DisplaySettings& settings) :
	mDisplaySettings(settings),
	mViewer(std::make_unique<osgViewer::CompositeViewer>()),
	mLoadTimingPolicy(LoadTimingPolicy::LoadAcrossMultipleFrames)
{
	assert(mViewer);

	forwardOsgLogToBoost();

	osg::DisplaySettings::instance()->setNumMultiSamples(settings.multiSampleCount);

	if (settings.texturePoolSizeBytes > std::numeric_limits<unsigned int>::max())
	{
		SKYBOLT_LOG(warning) << std::format("Texture pool size '{}' is too large for Open Scene Graph. Up to 4GB is supported.", settings.texturePoolSizeBytes);
	}
	unsigned int texturePoolSizeBytesUnsignedInt = unsigned int(std::min(settings.texturePoolSizeBytes, std::size_t(std::numeric_limits<unsigned int>::max())));
	osg::DisplaySettings::instance()->setMaxTexturePoolSize(texturePoolSizeBytesUnsignedInt);

	osg::setNotifyLevel(osg::WARN);
	mViewer->setKeyEventSetsDone(0); // disable default 'escape' key binding to quit the application
	mViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded); // TODO: Use multi-threaded?
}

bool VisRoot::render()
{
	if (mWindows.empty())
	{
		// Don't call Viewer::frame() if there are no windows,
		// because the viewer might not be initialized yet, and there is nothing to render.
		return true;
	}

	for (const auto& window : mWindows)
	{
		window->setLoadTimingPolicy(mLoadTimingPolicy);
	}

	mViewer->frame();

	return !mViewer->done();
}

osgViewer::ViewerBase& VisRoot::getViewer() const
{
	return *mViewer;
}

void VisRoot::addWindow(const WindowPtr& window)
{
	assert(!findFirst(mWindows, window));
	mViewer->addView(window->getView());
	mWindows.push_back(window);

	// Viewer can't be realized until at least one window exists.
	// It is safe to add windows after viewer is realized.
	if (!mViewer->isRealized())
	{
		mViewer->realize();
	}

	// FIXME: Workaround for OSG bug where maxTexturePoolSize is not set for graphics contexts created after viewer realize,
	// or for situations where OSG never calls realize() e.g embedded windows with external GL context.
	if (auto gc = window->getView()->getCamera()->getGraphicsContext(); gc && gc->getState())
	{
		size_t size = osg::DisplaySettings::instance()->getMaxTexturePoolSize();
		gc->getState()->setMaxTexturePoolSize(size);
	}
}

void VisRoot::removeWindow(const WindowPtr& window)
{
	eraseFirst(mWindows, window);
	mViewer->removeView(window->getView());
}

} // namespace vis
} // namespace skybolt