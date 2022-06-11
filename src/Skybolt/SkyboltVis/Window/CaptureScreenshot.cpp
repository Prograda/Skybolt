/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CaptureScreenshot.h"
#include "Window/Window.h"
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/WriteFile>

#include <filesystem>

namespace skybolt {
namespace vis {

class WriteToFile : public osgViewer::ScreenCaptureHandler::CaptureOperation
{
public:
    WriteToFile(const std::string& filename) :
		mFilename(filename)
	{
	}

    void operator() (const osg::Image& image, const unsigned int context_id) override
	{
		osgDB::writeImageFile(image, mFilename);
	}

protected:
    const std::string mFilename;
};

void captureScreenshot(Window& window, const std::string& filename)
{
	// Create directory if it does not already exist
	std::filesystem::path path(filename);
	if (!std::filesystem::exists(path.parent_path()))
	{
		std::filesystem::create_directories(path.parent_path());
	}

	// Capture
	osg::ref_ptr<WriteToFile> writeToFile = new WriteToFile(filename);
	osg::ref_ptr<osgViewer::ScreenCaptureHandler> screenCaptureHandler = new osgViewer::ScreenCaptureHandler(writeToFile);

	osgViewer::Viewer& viewer = window.getViewer();

	screenCaptureHandler->setFramesToCapture(1);
	screenCaptureHandler->captureNextFrame(viewer);
	window.render();
	screenCaptureHandler->stopCapture();
}

} // namespace vis
} // namespace skybolt
