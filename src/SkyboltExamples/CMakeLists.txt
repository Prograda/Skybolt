add_subdirectory (ExamplesCommon)
add_subdirectory (FlightSimApp)
add_subdirectory (MinimalApp)

OPTION(BUILD_SKYBOLT_QT "Build SkyboltQtApp")
if (BUILD_SKYBOLT_QT)
	add_subdirectory (SkyboltQtApp)
endif()