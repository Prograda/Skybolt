set(APP_NAME SkyboltPythonTests)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")
include_directories("../../")

find_package(Catch2 REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Development Interpreter)
find_package(pybind11 REQUIRED)

add_executable(${APP_NAME} ${SOURCE_FILES})

# Pass the pytest scripts directory as a compile definition so the C++ test knows where to find them
set(PYTEST_SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Scripts")
target_compile_definitions(${APP_NAME} PRIVATE
    PYTEST_SCRIPTS_DIR="${PYTEST_SCRIPTS_DIR}"
)

target_include_directories(${APP_NAME} PRIVATE
    ${Python3_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
)

target_link_libraries(${APP_NAME}
    Catch2::Catch2
    ${Python3_LIBRARIES}
    pybind11::embed
    SkyboltPython
)

set_target_properties(${APP_NAME} PROPERTIES FOLDER SkyboltPlugins)

catch_discover_tests(${APP_NAME})
