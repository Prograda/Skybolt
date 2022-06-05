set(TARGET_NAME JsbSim)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

find_package(JsbSim REQUIRED)
include_directories(${JSBSIM_INCLUDE_DIRS})

set(LIBS
SkyboltEngine
${JSBSIM_LIBRARIES}
ws2_32
)

add_library(${TARGET_NAME} ${SKYBOLT_PLUGINS_LIBRARY_TYPE} ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

target_include_directories(${TARGET_NAME} PUBLIC
	${JSBSIM_INCLUDE_DIR}
	${JSBSIM_INCLUDE_DIR}/JSBSim
)

set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE) #Export symbols for tests
set_engine_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})
