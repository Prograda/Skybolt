set(LIB_NAME SkyboltSim)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")

find_package(Boost COMPONENTS log REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

set(LIBRARIES
	${Boost_LIBRARIES}
	SkyboltCommon
	SkyboltReflection
)

target_link_libraries(${LIB_NAME} ${LIBRARIES})

skybolt_install(${LIB_NAME})