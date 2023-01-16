set(TARGET_NAME CigiComponent)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

find_package(cigicl REQUIRED)
include_directories(${cigicl_INCLUDE_DIR})

set(LIBS
SkyboltEngine
${cigicl_LIBRARIES}
)

add_library(${TARGET_NAME} ${SKYBOLT_PLUGINS_LIBRARY_TYPE} ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})


target_include_directories(${TARGET_NAME} PUBLIC
	${cigicl_INCLUDE_DIR}
)

set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE) #Export symbols for tests
set_engine_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})