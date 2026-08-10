/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "StaticOsgPlugins.h"
#include "VisObjectFactory.h"
#include "VisRoot.h"

#include <SkyboltCommon/Logging/Logging.h>
#include <SkyboltEngine/EngineRoot.h>
#include "SkyboltVisOsg/Camera.h"
#include "SkyboltVisOsg/OsgLogHandler.h"
#include "SkyboltVisOsg/RenderContext.h"
#include "SkyboltVisOsg/Scene.h"
#include "SkyboltVisOsg/SimVisBinding/SimVisSystem.h"
#include "SkyboltVisOsg/Renderable/Water/SimpleWaveHeightTextureGenerator.h"
#include "SkyboltVisOsg/RenderOperation/RenderTarget.h"
#include "SkyboltVisOsg/Window/Window.h"
#include <SkyboltVisOsg/Shader/ShaderProgramRegistry.h>
#include <SkyboltCommon/VectorUtility.h>

#include <osgDB/FileUtils>
#include <osgViewer/CompositeViewer>
#include <format>

namespace skybolt {
namespace vis {

static void addOsgFileSearchPaths(const std::vector<std::string>& assetPackagePaths)
{
	for (const std::string& path : assetPackagePaths)
	{
		osgDB::FilePathList& filePathList = osgDB::getDataFilePathList();
		filePathList.push_back(path);
	}
}

VisRoot::VisRoot(VisRootConfig config) :
	mDisplaySettings(std::move(config.displaySettings)),
	mViewer(std::make_unique<osgViewer::CompositeViewer>()),
	mLoadTimingPolicy(LoadTimingPolicy::LoadAcrossMultipleFrames)
{
	assert(mViewer);

	if (!config.world->getEntities().empty())
	{
		// Because VisRoot registers additional entity component factories,
		// we do a safety check here to ensure all the entities in the scene will be created with the expected components.
		throw std::runtime_error("VisRoot can only be created when no entities are in the scene");
	}

	addOsgFileSearchPaths(config.assetPackagePaths);

	ensureStaticOsgPluginsUsed();

	forwardOsgLogToBoost();

	osg::DisplaySettings::instance()->setNumMultiSamples(mDisplaySettings.multiSampleCount);

	if (mDisplaySettings.texturePoolSizeBytes > std::numeric_limits<unsigned int>::max())
	{
		SKYBOLT_LOG(warning) << std::format("Texture pool size '{}' is too large for Open Scene Graph. Up to 4GB is supported.", mDisplaySettings.texturePoolSizeBytes);
	}
	unsigned int texturePoolSizeBytesUnsignedInt = (unsigned int)(std::min(mDisplaySettings.texturePoolSizeBytes, std::size_t(std::numeric_limits<unsigned int>::max())));
	osg::DisplaySettings::instance()->setMaxTexturePoolSize(texturePoolSizeBytesUnsignedInt);

	osg::setNotifyLevel(osg::WARN);
	mViewer->setKeyEventSetsDone(0); // disable default 'escape' key binding to quit the application
	mViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded); // TODO: Use multi-threaded?

	mPrograms = std::make_unique<vis::ShaderPrograms>(vis::createShaderPrograms());
	mScene.reset(new vis::Scene(new osg::StateSet()));

	config.systemRegistry->push_back(std::make_shared<SimVisSystem>(config.world, mScene));

	if (!config.visFactoryRegistry->getFirstItemOfType<WaveHeightTextureGeneratorFactory>())
	{
		// Register default factory if a WaveHeightTextureGeneratorFactory has not been registred yet
		config.visFactoryRegistry->addItem(std::make_shared<vis::SimpleWaveHeightTextureGeneratorFactory>());
	}

	mVisObjectFactory = std::make_unique<VisObjectFactory>(VisObjectFactoryConfig{
		.scene = mScene,
		.visFactoryRegistry = config.visFactoryRegistry.get(),
		.componentFactoryRegistry = config.componentFactoryRegistry,
		.programs = mPrograms.get()
		});
}

VisRoot::~VisRoot() = default;

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