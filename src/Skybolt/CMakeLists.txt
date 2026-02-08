add_subdirectory (AircraftHud)

OPTION(BUILD_MAP_FEATURES_CONVERTER "Build MapFeaturesConverter")
if (BUILD_MAP_FEATURES_CONVERTER)
	add_subdirectory (MapFeaturesConverter)
endif()

add_subdirectory (SkyboltCommon)
add_subdirectory (SkyboltCommonTests)
add_subdirectory (SkyboltEngine)

if (BUILD_PYTHON_BINDINGS)
	add_subdirectory (SkyboltPythonBindings)
endif()

add_subdirectory (SkyboltEnginePlugins)

add_subdirectory (SkyboltEngineTests)

add_subdirectory (SkyboltSim)
add_subdirectory (SkyboltSimTests)
add_subdirectory (SkyboltVis)
add_subdirectory (SkyboltVisTests)
add_subdirectory (TileMapGenerator)

OPTION(BUILD_WITH_QT "Build with Qt")
if (BUILD_WITH_QT)
	add_subdirectory (SkyboltEngineQt)
endif()
