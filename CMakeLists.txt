cmake_minimum_required (VERSION 3.13.0)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0074 NEW) # allow <PackageName>_ROOT variables
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0") 
	cmake_policy(SET CMP0111 OLD) # suppress pybind11 missing imports warnings
endif()

project(Skybolt)
set(PACKAGE_VERSION 1.3.1)

enable_testing()

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

# Allow user to overwrite install path.
# Solution from https://stackoverflow.com/questions/39481958/setting-cmake-install-prefix-from-cmakelists-txt-file
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	if(DEFINED ENV{CMAKE_INSTALL_PREFIX})
		set(CMAKE_INSTALL_PREFIX $ENV{CMAKE_INSTALL_PREFIX}/Skybolt CACHE PATH "Overriding CMAKE_INSTALL_PREFIX from environment variable" FORCE)
	endif()
endif()

#-------------------------------------------------------------
# Compiler flags
#-------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
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
# Build tree
#-------------------------------------------------------------
include(AddSourceGroup)
include(SetPluginTargetProperties)
include(Catch)
include(SkyboltInstall)

add_subdirectory (src/Skybolt)
add_subdirectory (src/SkyboltExamples)

OPTION(BUILD_SPROCKET "Build Sprocket")
if (BUILD_SPROCKET)
	add_subdirectory (src/Sprocket)
endif()

#-------------------------------------------------------------
# Installation
#-------------------------------------------------------------
# Install conan files (required by 'conan editable' feature. See https://docs.conan.io/en/latest/developing_packages/editable_packages.html)
if (EXISTS ${CMAKE_SOURCE_DIR}/conanfile.py) # File might not exist if during conan build. See https://github.com/conan-io/conan/issues/5415.
	install(FILES ${CMAKE_SOURCE_DIR}/conanfile.py DESTINATION .)
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Conan	DESTINATION .)

# Install headers
install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/src/Skybolt
	DESTINATION include
	FILES_MATCHING
	PATTERN "*.h*"
)

# Install cmake config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    SkyboltConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
	EXPORT SkyboltTargets
	NAMESPACE Skybolt::
    DESTINATION cmake
)

configure_file(CMake/SkyboltConfig.cmake.in SkyboltConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SkyboltConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/SkyboltConfigVersion.cmake"
	DESTINATION cmake
)