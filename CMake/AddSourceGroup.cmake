macro(add_source_group path list)

	STRING(REPLACE "./" "Source Files/" GROUP_NAME ${path})
	STRING(REPLACE "/" "\\" GROUP_NAME ${GROUP_NAME})
	file(GLOB TMP_FILES "${path}/*.h" "${path}/*.cpp" "${path}/*.cc")
	source_group(${GROUP_NAME} FILES ${TMP_FILES})

	set(${list} ${${list}} ${TMP_FILES})

endmacro()


# path must be relative
macro(add_source_group_tree path list)
	file(GLOB_RECURSE TMP_FILES "${path}/*")
	foreach(f ${TMP_FILES})
		get_filename_component(BASE_PATH ${f} PATH)

			if(IS_DIRECTORY ${BASE_PATH})
				if (NOT ${BASE_PATH} MATCHES "^.*.svn*")

					file(RELATIVE_PATH BASE_PATH "${CMAKE_CURRENT_LIST_DIR}/${path}" ${BASE_PATH})
					set(TMP_PATHS ${TMP_PATHS} "${path}/${BASE_PATH}")
				endif()
			endif()
	endforeach()

	list(REMOVE_DUPLICATES TMP_PATHS)

	foreach(f ${TMP_PATHS})
		add_source_group(${f} ${list})
	endforeach()
endmacro()
