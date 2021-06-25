
add_source_group_tree(. SOURCE)

include_directories("../")

add_library(AircraftHud STATIC ${SOURCE})

target_link_libraries(AircraftHud glm)

skybolt_install(AircraftHud)