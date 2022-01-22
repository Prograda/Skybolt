set(APP_NAME SkyboltVisTests)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")
include_directories("../../../Assets/Core/Shaders")
find_package(Catch2)

find_package(OpenGL REQUIRED) # CaptureTexture.cpp requires OpenGL to workaround an OSG limitation

add_definitions(-DCMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} -DSHADERS_SOURCE_DIR="${CMAKE_SOURCE_DIR}/Assets/Core/Shaders")

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries(${APP_NAME} SkyboltVis Catch2::Catch2 ${OPENGL_LIBRARIES})

catch_discover_tests(${APP_NAME})