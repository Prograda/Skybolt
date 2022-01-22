
add_source_group_tree(. SOURCE)

find_package(glm REQUIRED)

include_directories("../")

add_library(AircraftHud STATIC ${SOURCE})

target_link_libraries(AircraftHud glm::glm)

skybolt_install(AircraftHud)