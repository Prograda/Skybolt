macro(skybolt_install target)
	# Install target
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		RUNTIME_DEPENDENCY_SET SkyboltDependencies # Adds runtime dependencies to SkyboltDependencies target
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
	)
endmacro()

macro(skybolt_plugin_install target)
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin/plugins
	)
endmacro()

macro(skybolt_python_module_install target)
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
	)
endmacro()