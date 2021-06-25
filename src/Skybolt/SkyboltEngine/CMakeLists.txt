set(LIB_NAME SkyboltEngine)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")

find_package(Boost COMPONENTS log program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(LIBRARIES
	SkyboltCommon
	SkyboltSim
	SkyboltVis
	${Boost_LIBRARIES}
	${CMAKE_DL_LIBS} # Required by gcc
)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${LIB_NAME} ${LIBRARIES})

target_include_directories(${LIB_NAME} PUBLIC
	${Boost_INCLUDE_DIRS}
)

skybolt_install(SkyboltEngine)