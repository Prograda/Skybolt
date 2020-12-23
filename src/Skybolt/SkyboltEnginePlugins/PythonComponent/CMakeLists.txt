set(TARGET_NAME PythonComponent)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR})

find_package(pybind11 REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS})

set(LIBS
	SkyboltEngine
	${PYTHON_LIBRARY}
	pybind11::headers
)

add_library(${TARGET_NAME} SHARED ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

set_engine_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})
