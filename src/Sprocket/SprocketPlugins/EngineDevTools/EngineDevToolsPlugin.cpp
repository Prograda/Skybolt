/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Sprocket/EditorPlugin.h>
#include <Sprocket/QtUtil/QtMenuUtil.h>
#include <Sprocket/Viewport/OsgWidget.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Diagnostics/StatsDisplaySystem.h>
#include <SkyboltSim/System/System.h>
#include <SkyboltVis/VisRoot.h>
#include <SkyboltVis/RenderOperation/RenderTarget.h>
#include <SkyboltVis/Window/Window.h>
#include <SkyboltVis/RenderOperation/RenderCameraViewport.h>
#include <SkyboltVis/RenderOperation/RenderOperationUtil.h>
#include <SkyboltVis/Shader/ShaderSourceFileChangeMonitor.h>

#include <QMainWindow>
#include <QMenuBar>

#include <boost/config.hpp>
#include <boost/dll/alias.hpp>

using namespace skybolt;

class EngineDevToolsPlugin;

class EngineDevToolsSystem : public sim::System
{
public:
	EngineDevToolsSystem(std::function<void()> updatable) : mUpdatable(updatable) {}
	void updatePreDynamics(const sim::System::StepArgs& args) override
	{
		mUpdatable();
	}

	std::function<void()> mUpdatable;
};

class EngineDevToolsPlugin : public EditorPlugin
{
public:
	EngineDevToolsPlugin(const EditorPluginConfig& config) :
		mEngineRoot(config.engineRoot),
		mVisRoot(config.visRoot)
	{
		assert(mVisRoot);

		QMenuBar* menuBar = config.mainWindow->menuBar();
		QMenu* devMenu = new QMenu("Developer", menuBar);
		insertMenuBefore(*menuBar, "Tools", *devMenu);

		{
			QAction* action = devMenu->addAction("Viewport Stats");
			action->setCheckable(true);
			QObject::connect(action, &QAction::triggered, [this](bool visible) { setStatsDisplaySystemEnabled(visible); });
		}
		{
			QAction* action = devMenu->addAction("Viewport Textures");
			action->setCheckable(true);
			QObject::connect(action, &QAction::triggered, [this](bool visible) { setViewportTextureDisplayEnabled(visible); });
		}
		{
			QAction* action = devMenu->addAction("Live Shader Editing");
			action->setCheckable(true);
			QObject::connect(action, &QAction::triggered, [this](bool visible) { setLiveShaderEditingEnabled(visible); });
		}

		mEngineRoot->systemRegistry->push_back(std::make_shared<EngineDevToolsSystem>([this] {
			if (mShaderSourceFileChangeMonitor)
			{
				mShaderSourceFileChangeMonitor->update();
			}
		}));
	}

	~EngineDevToolsPlugin() override = default;

	void acquireViewport()
	{
		mWindow = mVisRoot->getWindows().empty() ? nullptr : mVisRoot->getWindows().front();
		if (mWindow)
		{
			auto ops = mWindow->getRenderOperationSequence().findOperationsOfType<vis::RenderCameraViewport>();
			mViewport = ops.empty() ? nullptr : ops.back();
		}

	}

	void setStatsDisplaySystemEnabled(bool enabled)
	{
		acquireViewport();
		if (mWindow && mViewport)
		{
			if (!mStatsDisplaySystem)
			{
				mStatsDisplaySystem = std::make_shared<StatsDisplaySystem>(&mVisRoot->getViewer(), mWindow->getView(), mViewport->getFinalRenderTarget()->getOsgCamera());
				mStatsDisplaySystem->setVisible(false);
				mEngineRoot->systemRegistry->push_back(mStatsDisplaySystem);
			}

			mStatsDisplaySystem->setVisible(enabled);
		}
	}

	void setViewportTextureDisplayEnabled(bool enabled)
	{
		acquireViewport();
		if (mWindow && mViewport)
		{
			if (enabled && !mRenderOperationVisualization)
			{
				mRenderOperationVisualization = vis::createRenderOperationVisualization(mViewport, mEngineRoot->programs);
				mWindow->getRenderOperationSequence().addOperation(mRenderOperationVisualization);
			}
			else if (!enabled && mRenderOperationVisualization)
			{
				mWindow->getRenderOperationSequence().removeOperation(mRenderOperationVisualization);
			}
		}
	}

	void setLiveShaderEditingEnabled(bool enabled)
	{
		mShaderSourceFileChangeMonitor.reset();
		if (enabled)
		{
			mShaderSourceFileChangeMonitor = std::make_unique<vis::ShaderSourceFileChangeMonitor>(mEngineRoot->programs);
		}
	}

private:
	EngineRoot* mEngineRoot;
	vis::VisRoot* mVisRoot;
	vis::WindowPtr mWindow;
	osg::ref_ptr<skybolt::vis::RenderCameraViewport> mViewport;
	std::shared_ptr<skybolt::StatsDisplaySystem> mStatsDisplaySystem;
	std::unique_ptr<skybolt::vis::ShaderSourceFileChangeMonitor> mShaderSourceFileChangeMonitor;
	osg::ref_ptr<skybolt::vis::RenderOperation> mRenderOperationVisualization;
};

namespace plugins {

	std::shared_ptr<EditorPlugin> createEditorPlugin(const EditorPluginConfig& config)
	{
		return std::make_shared<EngineDevToolsPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEditorPlugin,
		createEditorPlugin
	)
}
