
add_source_group_tree(. SOURCE_FILES)

find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED log)
find_package(Threads REQUIRED) # Older GNU and LLVM linkers require this explicit linkage for using std::thread

include_directories("../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(SkyboltCommon STATIC ${SOURCE_FILES})

set(LIBRARIES
	Boost::boost
	glm::glm
	nlohmann_json::nlohmann_json
	Threads::Threads
)

if (BUILD_WITH_BOOST_LOG) #SKYBOLT_USE_BOOST_LOG
	list(APPEND LIBRARIES Boost::log)
endif()

target_link_libraries(SkyboltCommon PUBLIC ${LIBRARIES})

target_compile_definitions(SkyboltCommon PUBLIC GLM_FORCE_RADIANS BOOST_ALL_NO_LIB)

skybolt_install(SkyboltCommon)