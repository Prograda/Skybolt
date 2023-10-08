
add_source_group_tree(. SOURCE_FILES)

include_directories("../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(SkyboltReflection STATIC ${SOURCE_FILES})

skybolt_install(SkyboltReflection)