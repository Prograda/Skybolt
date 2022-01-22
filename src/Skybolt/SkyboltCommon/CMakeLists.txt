
add_source_group_tree(. SOURCE_FILES)

find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories("../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(SkyboltCommon STATIC ${SOURCE_FILES})

target_include_directories(SkyboltCommon PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(SkyboltCommon PUBLIC glm::glm nlohmann_json::nlohmann_json)

target_compile_definitions(SkyboltCommon PUBLIC GLM_FORCE_RADIANS BOOST_ALL_NO_LIB)

skybolt_install(SkyboltCommon)