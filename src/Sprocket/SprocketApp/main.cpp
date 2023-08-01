/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Sprocket/ApplicationUtil.h>
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/EngineSettingsSerialization.h>
#include <Sprocket/MainWindow.h>
#include <Sprocket/SprocketModel.h>
#include <Sprocket/ThirdParty/DarkStyle.h>
#include <Sprocket/Viewport/OsgWidget.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRootFactory.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltEngine/GetExecutablePath.h>
#include <SkyboltSim/World.h>
#include <SkyboltCommon/Stringify.h>

using namespace skybolt;

class Application : public QApplication
{
public:
	Application(const std::vector<PluginFactory>& enginePluginFactories, const std::vector<EditorPluginFactory>& editorPluginFactories, int argc, char* argv[]) :
		QApplication(argc, argv)
	{
		QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/qtplugins");

		setStyle(new DarkStyle);

		QSettings settings(QApplication::applicationName());
		nlohmann::json engineSettings = readOrCreateEngineSettingsFile(settings);
		std::shared_ptr<EngineRoot> engineRoot = EngineRootFactory::create(enginePluginFactories, engineSettings);

		mMainWindow.reset(new MainWindow([&] {
			MainWindowConfig c;
			c.engineRoot = engineRoot;
			c.editorPluginFactories = editorPluginFactories;
			return c;
		}()));

		try
		{
			if (argc >= 2)
			{
				QString filename(argv[1]);
				mMainWindow->open(filename);
			}
			else
			{
				mMainWindow->newScenario();
			}
		}
		catch (...)
		{
			mMainWindow.reset();
			throw;
		}

		mMainWindow->showMaximized();
	}

	~Application()
	{
		mMainWindow.reset();
	}

private:
	std::unique_ptr<MainWindow> mMainWindow;
};

int main(int argc, char *argv[])
{
	try
	{
		QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
		QCoreApplication::setApplicationName("Sprocket");

		{
			QSurfaceFormat format;
			format.setSamples(4);

#ifndef CONVERT_TO_SRGB_IN_SHADER
			format.setColorSpace(QSurfaceFormat::sRGBColorSpace);
#endif
			format.setDepthBufferSize(24);

			format.setProfile(QSurfaceFormat::CompatibilityProfile);

			QSurfaceFormat::setDefaultFormat(format);
		}

		std::string pluginsDir = (getExecutablePath() / "plugins").string();
		std::vector<PluginFactory> enginePluginFactories = loadPluginFactories<Plugin, PluginConfig>(pluginsDir);
		std::vector<EditorPluginFactory> editorPluginFactories = loadPluginFactories<EditorPlugin, EditorPluginConfig>(pluginsDir);
		Application application(enginePluginFactories, editorPluginFactories, argc, argv);

		if (!applicationSupportsOpenGl())
		{
			displayApplicationError("This program requires OpenGL to run.");
			return 1;
		}

		return application.exec();
	}
	catch (const std::exception &e)
	{
		displayApplicationError(e.what());
	}
	catch (...)
	{
		displayApplicationError("Exception caught in main");
	}

	return 0; 
}
