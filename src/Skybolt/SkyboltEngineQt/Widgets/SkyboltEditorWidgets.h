#pragma once

#include <SkyboltWidgets/Property/DefaultEditorWidgets.h>
#include <SkyboltWidgets/Property/PropertyEditorWidgetFactory.h>

std::unique_ptr<skybolt::PropertyEditorWidgetFactoryMap> createSkyboltEditorWidgetFactoryMap(const skybolt::DefaultEditorWidgetFactoryMapConfig& config);