set(TARGET_NAME skybolt)

add_source_group_tree(. SrcFiles)

include_directories("../")

find_package(Python3 REQUIRED COMPONENTS Development Interpreter)
include_directories(${Python3_INCLUDE_DIRS})

find_package(pybind11 REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS})

set(LIBS
	SkyboltEngine
	${Python3_LIBRARIES}
	pybind11::headers
)

include(pybind11Tools)
pybind11_add_module(${TARGET_NAME} ${SrcFiles} SHARED)
target_link_libraries (${TARGET_NAME} PUBLIC ${LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER PythonLibs)

skybolt_python_module_install(${TARGET_NAME})