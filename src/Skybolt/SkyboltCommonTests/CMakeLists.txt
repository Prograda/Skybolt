set(APP_NAME SkyboltCommonTests)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")

find_package(Catch2 REQUIRED)

# TODO: Remove this dependency. Common should not need to depend on osg.
find_package(OSG REQUIRED COMPONENTS osg)
include_directories(${OSG_INCLUDE_DIR})

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries (${APP_NAME} PUBLIC SkyboltCommon Catch2::Catch2)

catch_discover_tests(${APP_NAME})