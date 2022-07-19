#pragma once

#include <SkyboltVis/Rect.h>
#include <SkyboltVis/Window/StandaloneWindow.h>
#include <osgViewer/Viewer>

namespace skybolt {

std::unique_ptr<vis::StandaloneWindow> createExampleWindow()
{
	auto window = std::make_unique<vis::StandaloneWindow>(vis::RectI(0, 0, 1080, 720));
	window->getViewer().setKeyEventSetsDone(osgGA::GUIEventAdapter::KEY_Escape);
	return window;
}

} // namespace skybolt
