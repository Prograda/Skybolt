set(TARGET_NAME PythonConsole)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5 COMPONENTS Core REQUIRED)

find_package(QCodeEditor REQUIRED)
include_directories(${QCodeEditor_INCLUDE_DIR})

set(LIBS
Sprocket
${QCodeEditor_LIBRARIES}
)

add_library(${TARGET_NAME} SHARED ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

set_editor_plugin_target_properties(${TARGET_NAME})