macro(skybolt_install target)
	# Install target
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		RUNTIME_DEPENDENCY_SET SkyboltDependencies # Adds runtime dependencies to SkyboltDependencies target
		LIBRARY DESTINATION lib COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development EXCLUDE_FROM_ALL
		RUNTIME DESTINATION bin COMPONENT Runtime
	)
endmacro()

macro(skybolt_plugin_install target)
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		LIBRARY DESTINATION lib/plugins COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development EXCLUDE_FROM_ALL
		RUNTIME DESTINATION bin/plugins COMPONENT Runtime
	)
endmacro()

macro(skybolt_python_module_install target)
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		LIBRARY DESTINATION lib COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development EXCLUDE_FROM_ALL
		RUNTIME DESTINATION bin COMPONENT Runtime
	)
endmacro()

function(skybolt_install_deps)
	cmake_parse_arguments(ARG "EXCLUDE_FROM_ALL" "COMPONENT;DESTINATION" "EXECUTABLES;LIBRARIES" ${ARGN})

	if (NOT CONAN_RUNTIME_LIB_DIRS)
		message(FATAL_ERROR "Function currently only works when used with the Conan CMakeDeps generator")
	endif()
	if (NOT ARG_DESTINATION)
		message(FATAL_ERROR "Missing required argument: 'DESTINATION'.")
	endif()
	
	set(_get_runtime_deps_args)
	if (NOT ARG_LIBRARIES AND NOT ARG_EXECUTABLES)
		message(FATAL_ERROR "Need to specify at least one of 'LIBRARIES' or 'EXECUTABLES'")
	endif()
	if (ARG_LIBRARIES)
		list(APPEND _get_runtime_deps_args " LIBRARIES ${ARG_LIBRARIES}\n")
	endif()
	if (ARG_EXECUTABLES)
		list(APPEND _get_runtime_deps_args " EXECUTABLES ${ARG_EXECUTABLES}\n")
	endif()

	string(CONCAT _get_runtime_deps_code [=[
		file(GET_RUNTIME_DEPENDENCIES
			]=] "${_get_runtime_deps_args}"
		[=[
			RESOLVED_DEPENDENCIES_VAR resolved
			UNRESOLVED_DEPENDENCIES_VAR unresolved
			CONFLICTING_DEPENDENCIES_PREFIX conflicting
			PRE_EXCLUDE_REGEXES
				[[api-ms-]] # VC Redistibutable DLLs
				[[ext-ms-]] # Windows extension DLLs
				[[kernel32\.dll]]
				[[libc\.so\..*]] [[libgcc_s\.so\..*]] [[libm\.so\..*]] [[libstdc\+\+\.so\..*]]
			POST_EXCLUDE_REGEXES
				[[.*system32\/.*\.dll]] # Windows system DLLs
				[[^\/(lib|usr\/lib|usr\/local\/lib)]] # Unix system libraries
			DIRECTORIES
		]=] 
				"${CONAN_RUNTIME_LIB_DIRS}\n"
		[=[
		)
        if(0)
            message("file(GET_RUNTIME_DEPENDENCIES ]=] "${_get_runtime_deps_args}" [=[ ...)")
            foreach(_file IN LISTS resolved)
                message("    resolved: ${_file}")
            endforeach()
            foreach(_file IN LISTS unresolved)
                message("    unresolved: ${_file}")
            endforeach()
            foreach(_file IN LISTS conflicting_FILENAMES)
                message("    conflicting: ${_file}")
                message("    with ${conflicting_${_file}}")
            endforeach()
        endif()

		file(INSTALL ${resolved}
			DESTINATION "${CMAKE_INSTALL_PREFIX}/]=] "${ARG_DESTINATION}" [=["
			FOLLOW_SYMLINK_CHAIN
		)
		]=]
	)
	#message(FATAL_ERROR "_get_runtime_deps_code = ${_get_runtime_deps_code}")

	set(_install_args)
	if (ARG_COMPONENT)
		list(APPEND _install_args COMPONENT ${ARG_COMPONENT})
	endif()
	if (ARG_EXCLUDE_FROM_ALL)
		list(APPEND _install_args EXCLUDE_FROM_ALL)
	endif()

	install(CODE "${_get_runtime_deps_code}" ${_install_args})
endfunction()

# Install Qt plugins, which are not tracked by CMake runtime dependency system
function(_skybolt_install_qt_plugin_folder plugin_folder_name)
	set(_install_args ${ARGN})

	# FIXME: This pattern is duplicated a fair bit...
	set(_config_types ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
	list(REMOVE_DUPLICATES _config_types)
	foreach(_config ${_config_types})
		string(TOUPPER ${_config} _config_upper)

		install(
			DIRECTORY ${qt_PACKAGE_FOLDER_${_config_upper}}/plugins/${plugin_folder_name}
			CONFIGURATIONS ${_config}
			${_install_args}
			FILES_MATCHING PATTERN "*"
		)
	endforeach()
endfunction()

function(skybolt_install_qt_plugins)
	# FIXME: This only works with the Qt package provided by Conan using the CMakeDeps
	#   generator...

	# FIXME: Using the in-built CMake Qt-deployment functionality would be great.
	#	One downside is that it requires an executable to search for plugin usage,
	#   which means that it needs to be run over *ever* executable to capture all 
	#   plugins that may be used...  Also - at the time of writing, it doesn't 
	#   seem to work with the version of Qt provided by Conan - which fails to
	#   produce an environment variable specifying *where* the Qt package path is.
	#
	#qt_generate_deploy_app_script(
	#    TARGET ${TARGET_NAME}_qt_app
	#    OUTPUT_SCRIPT skybolt_install_qt_plugins_script
	#	NO_COMPILER_RUNTIME # Don't install compiler runtime; we do this elsewhere
	#	VERBOSE
	#)
	#install(SCRIPT "${skybolt_install_qt_plugins_script}" COMPONENT QtPlugins EXCLUDE_FROM_ALL)
	cmake_parse_arguments(ARG "EXCLUDE_FROM_ALL" "COMPONENT;DESTINATION" "" ${ARGN})

	if (NOT ARG_DESTINATION)
		set(ARG_DESTINATION bin/qtPlugins)
	endif()
	set(_install_args)
	if (ARG_COMPONENT)
		list(APPEND _install_args COMPONENT ${ARG_COMPONENT})
	endif()
	if (ARG_EXCLUDE_FROM_ALL)
		list(APPEND _install_args EXCLUDE_FROM_ALL)
	endif()

	# TODO: We should probably just install whatever packages exist
	_skybolt_install_qt_plugin_folder(generic DESTINATION "${ARG_DESTINATION}" ${_install_args})
	_skybolt_install_qt_plugin_folder(imageformats DESTINATION "${ARG_DESTINATION}" ${_install_args})
	_skybolt_install_qt_plugin_folder(platforms DESTINATION "${ARG_DESTINATION}" ${_install_args})
	_skybolt_install_qt_plugin_folder(platforminputcontexts DESTINATION "${ARG_DESTINATION}" ${_install_args})
	_skybolt_install_qt_plugin_folder(xcbglintegrations DESTINATION "${ARG_DESTINATION}" ${_install_args})

	# dlls go into 'bin', whereas sos go into 'lib' by default
	set(_runtime_lib_path ${CMAKE_INSTALL_LIBDIR})
	if (WIN32)
		set(_runtime_lib_path ${CMAKE_INSTALL_BINDIR})
	endif()

	# On Linux platform/libxcb.so dynamically loads *another* dependency!?!  Install it here:
	
	# FIXME: This pattern is duplicated a fair bit...
	set(_config_types ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
	list(REMOVE_DUPLICATES _config_types)
	foreach(_config ${_config_types})
		string(TOUPPER ${_config} _config_upper)

		file(GLOB _xcb_files "${qt_PACKAGE_FOLDER_${_config_upper}}/lib/libQt?XcbQpa${CMAKE_SHARED_LIBRARY_SUFFIX}*")
		if (_xcb_files)
			install(
				FILES ${_xcb_files}
				CONFIGURATIONS ${_config}
				# This is a general dependency (albeit depended-on by a plugin), so it goes in the 
				# runtime library directory; not the plugin directory
				DESTINATION "${_runtime_lib_path}"
				${_install_args}
			)
		endif()
	endforeach()
endfunction()


function(skybolt_install_osg_plugins)
	# FIXME: This only works with the Osg package provided by Conan using the CMakeDeps
	#   generator...
	cmake_parse_arguments(ARG "EXCLUDE_FROM_ALL" "COMPONENT" "" ${ARGN})

	if (NOT OpenSceneGraph_VERSION_STRING)
		message(FATAL_ERROR "OpenSceneGraph_VERSION_STRING not found")
	endif()

	set(_install_args)
	if (ARG_COMPONENT)
		list(APPEND _install_args COMPONENT ${ARG_COMPONENT})
	endif()
	if (ARG_EXCLUDE_FROM_ALL)
		list(APPEND _install_args EXCLUDE_FROM_ALL)
	endif()

	cmake_path(GET openscenegraph-mr_PACKAGE_FOLDER_RELWITHDEBINFO PARENT_PATH _osg_package_folder) # Get OSG package folder

	# dlls go into 'bin', whereas sos go into 'lib' by default
	set(_runtime_lib_path ${CMAKE_INSTALL_LIBDIR})
	if (WIN32)
		set(_runtime_lib_path ${CMAKE_INSTALL_BINDIR})
	endif()

	# FIXME: This pattern is duplicated a fair bit...
	set(_config_types ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
	list(REMOVE_DUPLICATES _config_types)
	foreach(_config ${_config_types})
		string(TOUPPER ${_config} _config_upper)

		set(_osg_plugins_dir "${openscenegraph-mr_PACKAGE_FOLDER_${_config_upper}}/${_runtime_lib_path}/osgPlugins-${OpenSceneGraph_VERSION_STRING}")

		file(GLOB_RECURSE _plugins "${_osg_plugins_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
		install(
			FILES ${_plugins}
			CONFIGURATIONS "${_config}"
			DESTINATION "${_runtime_lib_path}/osgPlugins-${OpenSceneGraph_VERSION_STRING}"
			${_install_args}
		)

		# Install the dependencies of the plugins
		skybolt_install_deps(
			LIBRARIES "${_plugins}" 
			DESTINATION "${_runtime_lib_path}"
			${_install_args}
		)
	endforeach()
endfunction()
