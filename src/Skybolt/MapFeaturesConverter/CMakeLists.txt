
add_source_group_tree(. SOURCE)

include_directories("../")

find_package(READOSM REQUIRED)
include_directories(${READOSM_INCLUDE_DIR})

find_package(ZLIB REQUIRED)

add_executable(MapFeaturesConverter ${SOURCE})

target_link_libraries (MapFeaturesConverter ${READOSM_LIBRARIES} ${ZLIB_LIBRARIES} SkyboltEngine SkyboltVis)