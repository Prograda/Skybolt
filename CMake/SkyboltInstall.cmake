macro(skybolt_install target)
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
	)
endmacro()