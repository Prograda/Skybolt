set(TARGET_NAME Sprocket)
add_source_group_tree(. SrcFiles)

FILE(GLOB UiSrc "*.ui")

include_directories("../")
include_directories("../../Skybolt")

find_package(OIS REQUIRED)
include_directories(${OIS_INCLUDE_DIR})

# Qt5
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

QT5_ADD_RESOURCES(SrcRccsSrc ${SrcRccs})

QT5_WRAP_UI(SrcUiHeaders ${UiSrc})
SET(SrcFiles ${SrcFiles} ${SrcUiHeaders} ${SrcRccsSrc})
SOURCE_GROUP("Generated Ui Files" FILES ${SrcUiHeaders} ${SrcRccsSrc})

find_package(ToolWindowManager REQUIRED)
include_directories(${ToolWindowManager_INCLUDE_DIR})

set(LIBS
	SkyboltEngine
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::OpenGL
	${ToolWindowManager_LIBRARIES}
	${OIS_LIBRARIES}
)

add_library(${TARGET_NAME} STATIC ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

target_include_directories(${TARGET_NAME} PUBLIC
	${ToolWindowManager_INCLUDE_DIR}
)

skybolt_install(${TARGET_NAME})