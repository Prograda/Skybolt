function(set_plugin_target_properties target folder)
	# In single config, the $<CONFIG> variable is built into the ${CMAKE_BINARY_DIR}, whereas in 
	# multi-config generators, it isn't - and needs to be added to the path
	get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	set_target_properties(${target} PROPERTIES 
		FOLDER ${folder}
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib$<$<BOOL:${_is_multi_config}>:/$<CONFIG>>/plugins"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib$<$<BOOL:${_is_multi_config}>:/$<CONFIG>>/plugins"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin$<$<BOOL:${_is_multi_config}>:/$<CONFIG>>/plugins"
	)
endfunction()

function(set_engine_plugin_target_properties target)
	set_plugin_target_properties(${target} SkyboltPlugins)
endfunction()
