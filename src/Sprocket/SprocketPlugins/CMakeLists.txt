add_subdirectory(EngineDevTools)
add_subdirectory(EntityController)
add_subdirectory(ViewportOverlays)

OPTION(BUILD_SEQUENCE_EDITOR_PLUGIN "Build SequenceEditor Plugin")
if (BUILD_SEQUENCE_EDITOR_PLUGIN)
	add_subdirectory(SequenceEditor)
endif()