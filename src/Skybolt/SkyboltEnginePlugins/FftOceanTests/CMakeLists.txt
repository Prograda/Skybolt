set(APP_NAME FftOceanTests)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")
include_directories("../../")

find_package(catch)

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries (${APP_NAME} FftOcean catch)

set_target_properties(${APP_NAME} PROPERTIES FOLDER SkyboltPlugins)

catch_discover_tests(${APP_NAME})