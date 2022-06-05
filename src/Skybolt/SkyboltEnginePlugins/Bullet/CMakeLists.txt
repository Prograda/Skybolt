set(TARGET_NAME Bullet)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

find_package(BULLET REQUIRED)
include_directories(${BULLET_INCLUDE_DIRS})
add_definitions(-DBT_USE_DOUBLE_PRECISION)

set(LIBS
SkyboltEngine
${BULLET_LIBRARIES}
)

add_library(${TARGET_NAME} ${SKYBOLT_PLUGINS_LIBRARY_TYPE} ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

target_include_directories(${TARGET_NAME} PUBLIC
	${BULLET_INCLUDE_DIRS}
)

set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE) #Export symbols for tests
set_engine_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})

if(MSVC AND ${MSVC_VERSION} GREATER_EQUAL 1915)
  # You must acknowledge that you understand MSVC resolved a byte alignment issue in this compiler
  # We get this due to using Eigen objects and allocating those objects with make_shared
  # TODO: Fix
  target_compile_definitions( ${TARGET_NAME} PUBLIC _DISABLE_EXTENDED_ALIGNED_STORAGE )
endif()
