
add_source_group_tree(. SOURCE)

include_directories("../")

add_executable(TileMapGenerator ${SOURCE})

target_link_libraries (TileMapGenerator SkyboltEngine)
