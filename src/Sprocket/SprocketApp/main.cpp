/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <Sprocket/UiApplication.h>
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/MainWindow.h>
#include <Sprocket/SprocketModel.h>
#include <Sprocket/OsgWidget.h>
#include <Sprocket/ThirdParty/DarkStyle.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltSim/World.h>
#include <SkyboltCommon/Stringify.h>

#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#pragma pop_macro("slots")

using namespace skybolt;

namespace py = pybind11;



static std::string appendConfigurationDir(const std::string& dir)
{
	return dir + "/" + CMAKE_INTDIR;
}

static std::string getLibDirectory()
{
	std::string dir = STRINGIFY(CMAKE_BINARY_DIR) "/lib";
	dir = appendConfigurationDir(dir);
	return dir;
}

static std::string getBinDirectory()
{
	std::string dir = STRINGIFY(CMAKE_BINARY_DIR) "/bin";
	dir = appendConfigurationDir(dir);
	return dir;
}

class Application : public UiApplication
{
public:
	Application(const std::vector<PluginFactory>& enginePluginFactories, const std::vector<EditorPluginFactory>& editorPluginFactories, int argc, char* argv[]) :
		UiApplication(argc, argv)
	{
		QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/qtplugins");

		setStyle(new DarkStyle);
		mMainWindow.reset(new MainWindow(enginePluginFactories, editorPluginFactories));
		mPyInterpreter.reset(new py::scoped_interpreter);

		try
		{
			py::list sysPath = py::module::import("sys").attr("path").cast<py::list>();
			sysPath.append(getLibDirectory());

			auto scriptFolders = getPathsInAssetPackages(mMainWindow->getEngineRoot()->getAssetPackagePaths(), "Scripts");
			for (const file::Path& scriptFolder : scriptFolders)
			{
				sysPath.append(scriptFolder);
			}

			py::module flow = py::module::import("skybolt");
			flow.attr("setGlobalEngineRoot")(mMainWindow->getEngineRoot());
			
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
		catch (const pybind11::error_already_set& e)
		{
			// Convert python exception to standard one because it will become invalid after the interpreter is destroyed.
			throw std::runtime_error(e.what());
		}
		catch (...)
		{
			mMainWindow.reset();
			mPyInterpreter.reset();
			throw;
		}

		mMainWindow->showMaximized();
	}

	~Application()
	{
		mMainWindow.reset();
		mPyInterpreter.reset();
	}

private:
	std::unique_ptr<py::scoped_interpreter> mPyInterpreter;
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

		std::string pluginsDir = getBinDirectory() + "/plugins";
		std::vector<PluginFactory> enginePluginFactories = loadPluginFactories<Plugin, PluginConfig>(pluginsDir);
		std::vector<EditorPluginFactory> editorPluginFactories = loadPluginFactories<EditorPlugin, EditorPluginConfig>(pluginsDir);
		Application application(enginePluginFactories, editorPluginFactories, argc, argv);

		if (!UiApplication::supportsOpenGl())
		{
			UiApplication::printError("This program requires OpenGL to run.");
			return 1;
		}

		return application.exec();
	}
	catch (const std::exception &e)
	{
		UiApplication::printError(e.what());
	}
	catch (...)
	{
		UiApplication::printError("Exception caught in main");
	}

	return 0; 
}
