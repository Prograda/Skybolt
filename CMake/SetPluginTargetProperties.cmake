macro(set_plugin_output_directory target directoryType baseDirectoryPath)
	set_target_properties( ${target}
	  PROPERTIES
	  ${directoryType}_DEBUG "${baseDirectoryPath}/Debug/plugins"
	  ${directoryType}_RELEASE "${baseDirectoryPath}/Release/plugins"
	  ${directoryType}_MINSIZEREL "${baseDirectoryPath}/MinSizeRel/plugins"
	  ${directoryType}_RELWITHDEBINFO "${baseDirectoryPath}/RelWithDebInfo/plugins"
	)
endmacro()

macro(set_plugin_target_properties target folder)

	set_target_properties(${target} PROPERTIES FOLDER ${folder})
	set_plugin_output_directory(${target} ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
	set_plugin_output_directory(${target} LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
	set_plugin_output_directory(${target} RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

endmacro()

macro(set_engine_plugin_target_properties target)
	set_plugin_target_properties(${target} SkyboltPlugins)
endmacro()
