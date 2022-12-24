/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CaptureScreenshot.h"
#include "VisRoot.h"

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/WriteFile>

#include <filesystem>

namespace skybolt {
namespace vis {

struct StoreImage : public osgViewer::ScreenCaptureHandler::CaptureOperation
{
    void operator() (const osg::Image& img, const unsigned int context_id) override
	{
		// Only capture from first context for now. TODO: write out one image per context.
		if (!image)
		{
			image = new osg::Image(img, osg::CopyOp::DEEP_COPY_ALL);
		}
	}

    osg::ref_ptr<osg::Image> image;
};

osg::ref_ptr<osg::Image> captureScreenshot(VisRoot& visRoot)
{
	osg::ref_ptr<StoreImage> storeImage = new StoreImage();
	osg::ref_ptr<osgViewer::ScreenCaptureHandler> screenCaptureHandler = new osgViewer::ScreenCaptureHandler(storeImage);

	screenCaptureHandler->setFramesToCapture(1);
	screenCaptureHandler->captureNextFrame(visRoot.getViewer());
	visRoot.render();
	screenCaptureHandler->stopCapture();

	return storeImage->image;
}

class WriteToFile : public osgViewer::ScreenCaptureHandler::CaptureOperation
{
public:
    WriteToFile(const std::string& filename) :
		mFilename(filename)
	{
	}

    void operator() (const osg::Image& image, const unsigned int context_id) override
	{
		// Only capture from first context for now. TODO: write out one image per context.
		if (!mCaptured)
		{
			osgDB::writeImageFile(image, mFilename);
			mCaptured = true;
		}
	}

protected:
    const std::string mFilename;
	bool mCaptured = false;
};

void captureScreenshot(VisRoot& visRoot, const std::string& filename)
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

	screenCaptureHandler->setFramesToCapture(1);
	screenCaptureHandler->captureNextFrame(visRoot.getViewer());
	visRoot.render();
	screenCaptureHandler->stopCapture();
}

} // namespace vis
} // namespace skybolt
