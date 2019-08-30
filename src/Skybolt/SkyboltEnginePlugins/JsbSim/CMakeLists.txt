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

add_library(${TARGET_NAME} SHARED ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

target_include_directories(${TARGET_NAME} PUBLIC
	${JSBSIM_INCLUDE_DIR}
	${JSBSIM_INCLUDE_DIR}/JSBSim
)

set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE) #Export symbols for tests
set_engine_plugin_target_properties(${TARGET_NAME})


if(MSVC AND ${MSVC_VERSION} GREATER_EQUAL 1915)
  # You must acknowledge that you understand MSVC resolved a byte alignment issue in this compiler
  # We get this due to using Eigen objects and allocating those objects with make_shared
  # TODO: Fix
  target_compile_definitions( ${TARGET_NAME} PUBLIC _DISABLE_EXTENDED_ALIGNED_STORAGE )
endif()
