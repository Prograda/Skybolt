set(LIB_NAME ExamplesCommon)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")
include_directories("../../Skybolt")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(LIBRARIES
	AircraftHud
	SkyboltEngine
)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(${LIB_NAME} PROPERTIES FOLDER Examples)

target_link_libraries(${LIB_NAME} ${LIBRARIES})
