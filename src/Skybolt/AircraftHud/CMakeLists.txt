
add_source_group_tree(. SOURCE)

include_directories("../")

find_package(Boost COMPONENTS thread date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_ALL_NO_LIB)

find_package(glm REQUIRED)

add_library(AircraftHud STATIC ${SOURCE})

target_link_libraries(AircraftHud glm)

skybolt_install(AircraftHud)