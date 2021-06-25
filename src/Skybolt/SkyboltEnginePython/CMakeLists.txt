set(TARGET_NAME skybolt)

add_source_group_tree(. SrcFiles)

include_directories("../")

find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)

find_package(pybind11 REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS})

set(LIBS
	SkyboltEngine
	${PYTHON_LIBRARY}
	pybind11::headers
)

include(pybind11Tools)
pybind11_add_module(${TARGET_NAME} ${SrcFiles})
target_link_libraries (${TARGET_NAME} PUBLIC ${LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER PythonLibs)
