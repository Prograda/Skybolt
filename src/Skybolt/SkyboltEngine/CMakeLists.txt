set(LIB_NAME SkyboltEngine)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")

find_package(Boost COMPONENTS log program_options REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(LIBRARIES
	SkyboltCommon
	SkyboltSim
	SkyboltVis
	Boost::boost
	Boost::log
	Boost::program_options
	${CMAKE_DL_LIBS} # Required by gcc
)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${LIB_NAME} ${LIBRARIES})

skybolt_install(SkyboltEngine)