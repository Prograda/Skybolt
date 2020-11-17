cmake_minimum_required (VERSION 2.8.3)
project(Skybolt)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/" "${CMAKE_SOURCE_DIR}/CMake/Modules/")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

OPTION(Boost_STATIC_LIBS "Boost use static libs")
if (Boost_STATIC_LIBS)
	set(Boost_USE_STATIC_LIBS ON)
endif()

cmake_policy(SET CMP0020 NEW)

include(AddSourceGroup)
include(SetPluginTargetProperties)
include(Catch)

add_subdirectory (src/Skybolt)
add_subdirectory (src/SkyboltExamples)

OPTION(BUILD_SPROCKET "Build Sprocket")
if (BUILD_SPROCKET)
	add_subdirectory (src/Sprocket)
endif()
