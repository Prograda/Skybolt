#include "WindowUtil.h"

#include <SkyboltVis/Rect.h>
#include <SkyboltVis/Window/StandaloneWindow.h>
#include <osgViewer/Viewer>

namespace skybolt {

std::unique_ptr<vis::StandaloneWindow> createExampleWindow()
{
	return std::make_unique<vis::StandaloneWindow>(vis::RectI(0, 0, 1080, 720));
}

} // namespace skybolt
