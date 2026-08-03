cmake_minimum_required (VERSION 3.26.0)

project(Skybolt)

include(CTest)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_SOURCE_DIR}/CMake/"
	"${CMAKE_SOURCE_DIR}/CMake/Modules/"
	"${CMAKE_BINARY_DIR}"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

OPTION(SKYBOLT_PLUGINS_STATIC_BUILD "Build skybolt plugins as static libraries" OFF)
if (SKYBOLT_PLUGINS_STATIC_BUILD)
	set (SKYBOLT_PLUGINS_LIBRARY_TYPE STATIC)
else()
	set (SKYBOLT_PLUGINS_LIBRARY_TYPE SHARED)
endif()

#-------------------------------------------------------------
# Compiler flags
#-------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

OPTION(DISABLE_OPTIMIZATION_RELWITHDEBINFO "Disable Optimization in RelWithDebInfo")
if(DISABLE_OPTIMIZATION_RELWITHDEBINFO)
	if(WIN32)
	set(OPT_OFF Od)
	else()
	set(OPT_OFF O0)
	endif()

	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-${OPT_OFF} -Zi")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-${OPT_OFF} -Zi")
endif()

if ( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4715") # Treat missing return as error
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5033") # disable warnings about use of 'register' in glm
	add_definitions(-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING) # disable deprecated member warnings from boost
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) # disable warnings about 'unsafe' functions
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register") # disable warnings about use of 'register' in glm
endif()

if(Skybolt_VERSION)
	add_definitions(-DSKYBOLT_VERSION=${Skybolt_VERSION})
endif()

#-------------------------------------------------------------
# Dependency options
#-------------------------------------------------------------
OPTION(Boost_STATIC_LIBS "Boost use static libs")
if (Boost_STATIC_LIBS)
	set(Boost_USE_STATIC_LIBS ON)
endif()

OPTION(OSG_STATIC_LIBS "OSG use static libs")
if (OSG_STATIC_LIBS)
	add_definitions(-DOSG_LIBRARY_STATIC)
endif()

#-------------------------------------------------------------
# Build options
#-------------------------------------------------------------

OPTION(BUILD_PYTHON_BINDINGS "Build Python Bindings")

# This option is provided to allow the application to opt-out of using boost as the logging backend,
# in cases where we want to minimize the number of dependencies. If this option is disabled, the application will
# fall back to a simple logging backend that writes to stdout.
OPTION(BUILD_WITH_BOOST_LOG "Use Boost as the logging backend" ON)
if (BUILD_WITH_BOOST_LOG)
	add_definitions(-DUSE_BOOST_LOG)
endif()

#-------------------------------------------------------------
# Build tree
#-------------------------------------------------------------
include(AddSourceGroup)
include(SetPluginTargetProperties)
include(Catch)
include(SkyboltInstall)

add_subdirectory(src/Skybolt)

option(SKYBOLT_BUILD_EXAMPLES "Build Skybolt Examples" ON)
add_subdirectory(src/SkyboltExamples)

function(_skybolt_configure_conan_init_script)
	# TODO: Document this
	set(_runtime_lib_path ${CMAKE_INSTALL_LIBDIR})
	if (WIN32)
		set(_runtime_lib_path ${CMAKE_INSTALL_BINDIR})
	endif()

	# FIXME: This'll probably fail on Windows with a multi-generator
	set(Skybolt_PLUGIN_DIRS "\"${CMAKE_BINARY_DIR}/${_runtime_lib_path}/plugins\"")
	configure_file(
		"CMake/conan-skybolt-init.cmake.in" 
		"${CMAKE_BINARY_DIR}/conan-skybolt-init-buildtree.cmake"
		@ONLY
	)	

	set(Skybolt_PLUGIN_DIRS "\"\${CMAKE_CURRENT_LIST_DIR}/../${_runtime_lib_path}/plugins\"")
	configure_file("CMake/conan-skybolt-init.cmake.in" "${CMAKE_BINARY_DIR}/conan-skybolt-init-installtree.cmake" @ONLY)
	install(FILES ${CMAKE_BINARY_DIR}/cmake-conan-skybolt-init-installtree.cmake
		DESTINATION CMake
		COMPONENT Development
		EXCLUDE_FROM_ALL
	)
endfunction()

_skybolt_configure_conan_init_script()

#-------------------------------------------------------------
# Installation
#-------------------------------------------------------------
# Install CMake scripts
install(DIRECTORY ${CMAKE_SOURCE_DIR}/CMake	
	DESTINATION .
	COMPONENT Development
	EXCLUDE_FROM_ALL
)

# Install headers
install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/src/Skybolt
	DESTINATION include
	COMPONENT Development
	EXCLUDE_FROM_ALL
	FILES_MATCHING PATTERN "*.h*"
)

# Install runtime dependencies
install(RUNTIME_DEPENDENCY_SET SkyboltDependencies
	PRE_EXCLUDE_REGEXES
		[=[api-ms-]=] # VC Redistibutable DLLs
		[=[ext-ms-]=] # Windows extension DLLs
		[[kernel32\.dll]]
		[[libc\.so\..*]] [[libgcc_s\.so\..*]] [[libm\.so\..*]] [[libstdc\+\+\.so\..*]]
	POST_EXCLUDE_REGEXES
		[=[.*system32\/.*\.dll]=] # Windows system DLLs
		[=[^\/(lib|usr\/lib|usr\/local\/lib)]=] # Unix system libraries
	DIRECTORIES ${CONAN_RUNTIME_LIB_DIRS}
	RUNTIME 
		COMPONENT SkyboltDependencies
		EXCLUDE_FROM_ALL
	LIBRARY
		COMPONENT SkyboltDependencies
		EXCLUDE_FROM_ALL
)

# Install assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Assets	
	DESTINATION .
	COMPONENT Assets
	EXCLUDE_FROM_ALL
)
