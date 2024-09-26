set(TARGET_NAME SkyboltQt)
add_source_group_tree(. SrcFiles)

FILE(GLOB UiSrc "*.ui")

include_directories("../")
include_directories("../../Skybolt")

# Qt5
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

QT5_ADD_RESOURCES(SrcRccsSrc ${SrcRccs})

QT5_WRAP_UI(SrcUiHeaders ${UiSrc})
SET(SrcFiles ${SrcFiles} ${SrcUiHeaders} ${SrcRccsSrc})
SOURCE_GROUP("Generated Ui Files" FILES ${SrcUiHeaders} ${SrcRccsSrc})

find_package(ToolWindowManager REQUIRED)

set(LIBS
	SkyboltEngine
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::OpenGL
	toolwindowmanager::toolwindowmanager
)

add_library(${TARGET_NAME} STATIC ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

skybolt_install(${TARGET_NAME})

# Install Qt plugins, which are not tracked by CMake runtime dependency system
install(
	DIRECTORY ${qt_PACKAGE_FOLDER_RELEASE}/plugins/platforms
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
	DESTINATION bin/qtPlugins
	FILES_MATCHING PATTERN "*"
)
install(
	DIRECTORY ${qt_PACKAGE_FOLDER_DEBUG}/plugins/platforms
    CONFIGURATIONS Debug
	DESTINATION bin/qtPlugins
	FILES_MATCHING PATTERN "*"
)
