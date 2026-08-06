set(TARGET_NAME FftOcean)
add_source_group_tree(. SrcFiles)

include_directories("./")
include_directories("../")
include_directories("../../")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(muFFT REQUIRED)

find_package(cxxtimer)
find_package(xsimd)

set(LIBS
	cxxtimer::cxxtimer
	mufft::mufft
	SkyboltEngine
	xsimd::xsimd
)

add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING) # disable deprecation warning for xsimd

add_library(${TARGET_NAME} ${SKYBOLT_PLUGINS_LIBRARY_TYPE} ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

# If using clang compiler, enable experimental library to allow std::execution::par and std::execution_seq to compile.
# Note that as of 2026, clang does not fully support these and may fall back to sequential execution.
target_compile_options(${TARGET_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fexperimental-library>
)

set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE) #Export symbols for tests
set_engine_plugin_target_properties(${TARGET_NAME})
skybolt_plugin_install(${TARGET_NAME})
