set(APP_NAME FlightSimApp)

add_source_group_tree(. SOURCE)


include_directories("../")
include_directories("../../Skybolt")

set(LIBS
ExamplesCommon
)

add_executable(${APP_NAME} ${SOURCE})
target_link_libraries (${APP_NAME} ${LIBS})

set_target_properties(${APP_NAME} PROPERTIES FOLDER Examples)

if(WIN32)
    set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
	set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
	set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
endif()
