set(TARGET_NAME SkyboltQtApp)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")
include_directories("../../Skybolt")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5 COMPONENTS Core REQUIRED)

SET(SrcRccs "resources/darkstyle.qrc")
QT5_ADD_RESOURCES(SrcRccsSrc ${SrcRccs})

SET(SOURCE_FILES ${SOURCE_FILES} ${SrcRccsSrc})
SOURCE_GROUP("Generated Ui Files" FILES ${SrcRccsSrc})

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries (${TARGET_NAME} SkyboltQt)

if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
	set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
endif()
