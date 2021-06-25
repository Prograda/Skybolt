add_subdirectory (AircraftHud)
add_subdirectory (MapAttributesConverter)

OPTION(BUILD_MAP_FEATURES_CONVERTER "Build MapFeaturesConverter")
if (BUILD_MAP_FEATURES_CONVERTER)
	add_subdirectory (MapFeaturesConverter)
endif()

add_subdirectory (SkyboltCommon)
add_subdirectory (SkyboltCommonTests)
add_subdirectory (SkyboltEngine)

OPTION(BUILD_PYTHON_BINDINGS "Build Python Bindings")
if (BUILD_PYTHON_BINDINGS)
	add_subdirectory (SkyboltEnginePython)
endif()

add_subdirectory (SkyboltEnginePlugins)

add_subdirectory (SkyboltEngineTests)
add_subdirectory (SkyboltSim)
add_subdirectory (SkyboltSimTests)
add_subdirectory (SkyboltVis)
add_subdirectory (SkyboltVisTests)
add_subdirectory (TileMapGenerator)
