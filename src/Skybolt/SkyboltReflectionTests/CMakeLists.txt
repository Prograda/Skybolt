set(APP_NAME SkyboltReflectionTests)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")

find_package(Catch2 REQUIRED)

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries (${APP_NAME} PUBLIC SkyboltReflection Catch2::Catch2)

catch_discover_tests(${APP_NAME})