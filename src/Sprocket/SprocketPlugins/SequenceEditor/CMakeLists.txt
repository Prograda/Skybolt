set(TARGET_NAME SequenceEditor)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

find_package(Qwt REQUIRED)
include_directories(${Qwt_INCLUDE_DIR})
add_definitions(-DQWT_DLL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5 COMPONENTS Core REQUIRED)

set(LIBS
Sprocket
${Qwt_LIBRARIES}
)

add_library(${TARGET_NAME} SHARED ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

set_editor_plugin_target_properties(${TARGET_NAME})