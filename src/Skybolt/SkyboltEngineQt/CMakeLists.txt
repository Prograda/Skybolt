set(TARGET_NAME SkyboltEngineQt)
add_source_group_tree(. SrcFiles)

FILE(GLOB UiSrc "*.ui")

include_directories("../")
include_directories("../../Skybolt")

find_package(skybolt-widgets)

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

QT6_ADD_RESOURCES(SrcRccsSrc ${SrcRccs})

QT6_WRAP_UI(SrcUiHeaders ${UiSrc})
SET(SrcFiles ${SrcFiles} ${SrcUiHeaders} ${SrcRccsSrc})
SOURCE_GROUP("Generated Ui Files" FILES ${SrcUiHeaders} ${SrcRccsSrc})

set(LIBS
	qt::qt
	SkyboltEngine
	SkyboltVisOsg
	skybolt-widgets::skybolt-widgets
)

add_library(${TARGET_NAME} STATIC ${SrcFiles})

if (UNIX)
	# On Linux when using the X11 Qt backend, OsgWindow fails to acquire a GL context 
	# correctly and causes a crash.  This hack launches the GL view in a separate 
	# window (which looks crap, but works) to workaround the issue.
	target_compile_definitions(${TARGET_NAME} PRIVATE SKYBOLT_QT_OSG_WINDOW_HACK)
endif()

target_link_libraries (${TARGET_NAME} ${LIBS})

skybolt_install(${TARGET_NAME})

skybolt_install_qt_plugins(COMPONENT QtPlugins EXCLUDE_FROM_ALL)

