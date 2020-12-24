set(APP_NAME SkyboltVisTests)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")

find_package(catch)

find_package(OpenGL REQUIRED) # CaptureTexture.cpp requires OpenGL to workaround an OSG limitation

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries(${APP_NAME} SkyboltVis catch ${OPENGL_LIBRARIES})

catch_discover_tests(${APP_NAME})