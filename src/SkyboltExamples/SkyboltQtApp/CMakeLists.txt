set(TARGET_NAME SkyboltQtApp)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")
include_directories("../../Skybolt")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Core REQUIRED)

set(SrcRccs "resources/darkstyle.qrc")
QT5_ADD_RESOURCES(SrcRccsSrc ${SrcRccs})

set(SOURCE_FILES ${SOURCE_FILES} ${SrcRccsSrc})
source_group("Generated Ui Files" FILES ${SrcRccsSrc})

# If python is found, define the python version in C++ so that the application
# can determine whether python is available at runtime.
if (BUILD_PYTHON_PLUGIN)
	find_package(Python3 QUIET COMPONENTS Interpreter)
	if (Python3_FOUND)
		# Parse the major and minor version numbers
		string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" _ ${Python3_VERSION})
		add_compile_definitions(PYTHON_VERSION_MAJOR=${CMAKE_MATCH_1})
		add_compile_definitions(PYTHON_VERSION_MINOR=${CMAKE_MATCH_2})
	endif()
endif()

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries (${TARGET_NAME} SkyboltQt)

if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
endif()

skybolt_install(${TARGET_NAME})