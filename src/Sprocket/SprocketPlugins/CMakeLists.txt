add_subdirectory(EntityController)

OPTION(BUILD_SEQUENCE_EDITOR_PLUGIN "Build SequenceEditor Plugin")
if (BUILD_SEQUENCE_EDITOR_PLUGIN)
	add_subdirectory(SequenceEditor)
endif()