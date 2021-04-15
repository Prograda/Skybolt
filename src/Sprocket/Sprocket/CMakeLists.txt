set(TARGET_NAME Sprocket)
add_source_group_tree(. SrcFiles)

FILE(GLOB UiSrc "*.ui")

include_directories("../")

find_package(OIS REQUIRED)
include_directories(${OIS_INCLUDE_DIR})

find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR})

find_package(pybind11 REQUIRED)

# Qt5
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

QT5_ADD_RESOURCES(SrcRccsSrc ${SrcRccs})

QT5_WRAP_UI(SrcUiHeaders ${UiSrc})
SET(SrcFiles ${SrcFiles} ${SrcUiHeaders} ${SrcRccsSrc})
SOURCE_GROUP("Generated Ui Files" FILES ${SrcUiHeaders} ${SrcRccsSrc})

find_package(ToolWindowManager)
include_directories(${ToolWindowManager_INCLUDE_DIR})

set(LIBS
	SkyboltEngine
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::OpenGL
	${ToolWindowManager_LIBRARIES}
	${OIS_LIBRARIES}
	${PYTHON_LIBRARY}
	pybind11::headers
)

add_library(${TARGET_NAME} STATIC ${SrcFiles})

add_dependencies(${TARGET_NAME} skybolt)

target_link_libraries (${TARGET_NAME} ${LIBS})

target_include_directories(${TARGET_NAME} PUBLIC
	${PYTHON_INCLUDE_DIR}
	${ToolWindowManager_INCLUDE_DIR}
	"${CMAKE_CURRENT_SOURCE_DIR}/../../Skybolt"
)