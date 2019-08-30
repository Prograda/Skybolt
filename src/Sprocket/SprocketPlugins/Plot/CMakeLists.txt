set(TARGET_NAME Plot)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

find_package(Qwt REQUIRED)
include_directories(${Qwt_INCLUDE_DIR})
add_definitions(-DQWT_DLL)

set(LIBS
Sprocket
${Qwt_LIBRARIES}
)

add_library(${TARGET_NAME} SHARED ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

set_editor_plugin_target_properties(${TARGET_NAME})