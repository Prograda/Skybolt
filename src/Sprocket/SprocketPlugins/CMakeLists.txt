add_subdirectory(EntityController)

OPTION(BUILD_NODE_GRAPH_PLUGIN "Build NodeGraph Plugin")
if (BUILD_NODE_GRAPH_PLUGIN)
	add_subdirectory(NodeGraph)
endif()

OPTION(BUILD_PLOT_PLUGIN "Build Plot Plugin")
if (BUILD_PLOT_PLUGIN)
	add_subdirectory(Plot)
endif()

OPTION(BUILD_PYTHON_CONSOLE_PLUGIN "Build PythonConsole Plugin")
if (BUILD_PYTHON_CONSOLE_PLUGIN)
	add_subdirectory(PythonConsole)
endif()

OPTION(BUILD_SEQUENCE_EDITOR_PLUGIN "Build SequenceEditor Plugin")
if (BUILD_SEQUENCE_EDITOR_PLUGIN)
	add_subdirectory(SequenceEditor)
endif()