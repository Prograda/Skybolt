
add_source_group_tree(. SOURCE)

include_directories("../")

find_package(readosm REQUIRED)
include_directories(${readosm_INCLUDE_DIR})

find_package(ZLIB REQUIRED)

add_executable(MapFeaturesConverter ${SOURCE})

target_link_libraries (MapFeaturesConverter ${readosm_LIBRARIES} ${ZLIB_LIBRARIES} SkyboltEngine SkyboltVis)