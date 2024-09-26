set(TARGET_NAME SkyboltPython)
add_source_group_tree(. SrcFiles)

include_directories("../")
include_directories("../../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Python3 REQUIRED COMPONENTS Development Interpreter)
include_directories(${Python3_INCLUDE_DIRS})

find_package(pybind11 REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS})

set(LIBS
	SkyboltEngine
	${Python3_LIBRARIES}
	pybind11::headers
)

add_library(${TARGET_NAME} ${SKYBOLT_PLUGINS_LIBRARY_TYPE} ${SrcFiles})

target_include_directories(${TARGET_NAME} PUBLIC ${Skybolt_INCLUDE_DIR}/Skybolt)
target_link_libraries (${TARGET_NAME} ${LIBS})

set_engine_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})
