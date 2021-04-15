/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CaptureScreenshot.h"
#include "Window/Window.h"
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/WriteFile>

namespace skybolt {
namespace vis {

struct MyFinalDrawCallback : public osg::Camera::DrawCallback
{
	void operator() (osg::RenderInfo& renderInfo) const override
	{
		auto viewport = renderInfo.getCurrentCamera()->getViewport();
		osg::ref_ptr<osg::Image> image = new osg::Image();
		//image->readPixels(0, 0, viewport->width(), viewport->height(), GL_RGBA, GL_UNSIGNED_BYTE);
		image->readImageFromCurrentTexture(renderInfo.getContextID(), false);
		osgDB::writeImageFile(*image, "test.png");
	}
};

void captureScreenshot(Window& window, const std::string& filename)
{
	osg::ref_ptr<osgViewer::ScreenCaptureHandler::WriteToFile> writeToFile = new osgViewer::ScreenCaptureHandler::WriteToFile(filename, ".png");
	osg::ref_ptr<osgViewer::ScreenCaptureHandler> screenCaptureHandler = new osgViewer::ScreenCaptureHandler(writeToFile);

	osgViewer::Viewer& viewer = window.getViewer();

	osg::ref_ptr<MyFinalDrawCallback> callback = new MyFinalDrawCallback();

	//screenCaptureHandler->setFramesToCapture(1);
	//screenCaptureHandler->captureNextFrame(*viewer);
	viewer.getCamera()->addFinalDrawCallback(callback);
	window.render();
	viewer.getCamera()->removeFinalDrawCallback(callback);
	//screenCaptureHandler->stopCapture();
}

} // namespace vis
} // namespace skybolt
