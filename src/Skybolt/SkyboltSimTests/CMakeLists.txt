set(APP_NAME SkyboltSimTests)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")

find_package(catch)

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries (${APP_NAME} SkyboltSim catch)

catch_discover_tests(${APP_NAME})