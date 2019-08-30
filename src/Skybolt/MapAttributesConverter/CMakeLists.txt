
add_source_group_tree(. SOURCE)

include_directories("../")

find_package(OSG REQUIRED)
include_directories(${OSG_INCLUDE_DIR})

add_library(MapAttributesConverter STATIC ${SOURCE})
