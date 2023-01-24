set(APP_NAME CigiTestHost)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")
include_directories("../../")

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries (${APP_NAME} CigiComponent)

set_target_properties(${APP_NAME} PROPERTIES FOLDER SkyboltPlugins)
