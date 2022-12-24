#include "WindowUtil.h"

#include <SkyboltVis/Rect.h>
#include <SkyboltVis/VisRoot.h>
#include <osgViewer/CompositeViewer>

namespace skybolt {

std::unique_ptr<vis::VisRoot> createExampleVisRoot()
{
	auto visRoot = std::make_unique<vis::VisRoot>();
	visRoot->getViewer().setKeyEventSetsDone(osgGA::GUIEventAdapter::KEY_Escape);
	return visRoot;
}

} // namespace skybolt
