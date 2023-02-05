set(TARGET_NAME EntityController)
add_source_group_tree(. SrcFiles)

include_directories("../../")
include_directories("../../../Skybolt")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5 COMPONENTS Core REQUIRED)

set(LIBS
Sprocket
)

add_library(${TARGET_NAME} SHARED ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

set_editor_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})