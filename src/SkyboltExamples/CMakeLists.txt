add_subdirectory (ExamplesCommon)
add_subdirectory (FlightSimApp)
add_subdirectory (MinimalApp)

if (BUILD_WITH_QT)
	add_subdirectory (ScenarioViewerApp)
endif()
