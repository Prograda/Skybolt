#pragma once

#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltSim/Spatial/LatLon.h>

Q_DECLARE_METATYPE(skybolt::sim::LatLon)

skybolt::ReflTypePropertyFactoryMap createSkyboltReflTypePropertyFactories(skybolt::refl::TypeRegistry& typeRegistry);