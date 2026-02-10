set(TARGET_NAME SkyboltJsbSim)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

find_package(jsbsim REQUIRED)

add_library(${TARGET_NAME} ${SKYBOLT_PLUGINS_LIBRARY_TYPE} ${SrcFiles})

target_include_directories(${TARGET_NAME} PUBLIC ${jsbsim_INCLUDE_DIR}/..)
target_link_libraries (${TARGET_NAME} SkyboltEngine jsbsim::jsbsim)

set_engine_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})
