set(TARGET_NAME NodeGraph)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

find_package(NodeEditor)
include_directories(${NodeEditor_INCLUDE_DIR})
add_definitions(-DNODE_EDITOR_SHARED)

find_package(QCodeEditor REQUIRED)
include_directories(${QCodeEditor_INCLUDE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5 COMPONENTS Core REQUIRED)

set(LIBS
Sprocket
${NodeEditor_LIBRARIES}
${QCodeEditor_LIBRARIES}
)

add_library(${TARGET_NAME} SHARED ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

set_editor_plugin_target_properties(${TARGET_NAME})