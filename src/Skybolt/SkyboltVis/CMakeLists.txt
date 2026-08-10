set(LIB_NAME SkyboltVis)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")

find_package(cxxtimer REQUIRED)
find_package(earcut_hpp REQUIRED)
find_package(httplib REQUIRED)
find_package(px_sched REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(LIBRARIES
	SkyboltCommon
	SkyboltSim
	${muFFT_LIBRARIES}
	cxxtimer::cxxtimer
	earcut_hpp::earcut_hpp
	httplib::httplib
	px_sched::px_sched
)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${LIB_NAME} PUBLIC ${OSG_INCLUDE_DIR})
target_link_libraries(${LIB_NAME} ${LIBRARIES})

skybolt_install(${LIB_NAME})
