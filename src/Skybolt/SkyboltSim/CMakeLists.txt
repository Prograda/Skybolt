set(LIB_NAME SkyboltSim)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

find_package(rttr REQUIRED)

set(LIBRARIES
	RTTR::Core_Lib
	SkyboltCommon
)

target_link_libraries(${LIB_NAME} ${LIBRARIES})

skybolt_install(${LIB_NAME})