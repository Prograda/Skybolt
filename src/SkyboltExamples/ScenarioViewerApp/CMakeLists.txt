set(TARGET_NAME ScenarioViewerApp)
add_source_group_tree(. SrcFiles)

FILE(GLOB UiSrc "*.ui")

set(SrcResources
    "Resources/Splash.png"
)

include_directories("../")
include_directories("../../Skybolt")

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

QT6_WRAP_UI(SrcUiHeaders ${UiSrc})

SET(SrcFiles ${SrcFiles} ${SrcUiHeaders} ${SrcResources})
SOURCE_GROUP("Generated Ui Files" FILES ${SrcUiHeaders})

# If python is found, define the python version in C++ so that the application
# can determine whether python is available at runtime.
if (BUILD_PYTHON_BINDINGS)
	find_package(Python3 REQUIRED COMPONENTS Interpreter)
	if (Python3_FOUND)
		# Parse the major and minor version numbers
		string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" _ ${Python3_VERSION})
		add_compile_definitions(PYTHON_VERSION_MAJOR=${CMAKE_MATCH_1})
		add_compile_definitions(PYTHON_VERSION_MINOR=${CMAKE_MATCH_2})
	endif()
endif()

set(LIBS
	SkyboltEngineQt
)

add_executable(${TARGET_NAME} ${SrcFiles})

if(WIN32)
    target_sources(${TARGET_NAME} PRIVATE ScenarioViewerApp.manifest)
endif()

QT6_ADD_RESOURCES(${TARGET_NAME} app_resources FILES ${SrcResources})

target_link_libraries (${TARGET_NAME} ${LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Examples)

skybolt_install(${TARGET_NAME})

if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
endif()
