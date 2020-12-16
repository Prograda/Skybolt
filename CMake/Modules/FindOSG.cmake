# - Try to find the OSG library
#
# Once done this will define
#
#  OSG_FOUND - system has OSG installed
#  OSG_INCLUDE_DIRS                   	OSG include directories, not cached
#  OSG_INCLUDE_DIR                    	This is almost the same as above, but this one is cached and may be modified by advanced users
#  OSG_LIBRARIES                      	Link these to use the OSG libraries that you specified, not cached
#  OSG_LIBRARY_DIRS                   	The path to where the OSG library files are.
#  OSG_MAJOR_VERSION
#  OSG_MINOR_VERSION
#  OSG_PATCH_VERSION
# OSG_SO_VERSION
# OSG_VERSION
#  OSG_${COMPONENT}_FOUND             		True IF the OSG library "component" was found.
#  OSG_${COMPONENT}_LIBRARY			The absolute path of the OSG library "component".
#  OSG_${COMPONENT}_LIBRARY_DEBUG    	The absolute path of the debug version of the  library "component".
#  OSG_${COMPONENT}_LIBRARY_RELEASE  	The absolute path of the release version of the OSG library "component"
#
# Some component aliases are available separately:
# 	LoaderLibs
# 	CoordSysLibs
#	MarineLibs
# 	ShaderLibs
#
# Mix n' match from FindBoost script

############################################
#
# Check the existence of the libraries.
#
############################################
# This macro was taken directly from the FindQt4.cmake file that is included
# with the CMake distribution. This is NOT my work. All work was done by the
# original authors of the FindQt4.cmake file. Only minor modifications were
# made to remove references to Qt and make this file more generally applicable
#########################################################################

MACRO (_OSG_ADJUST_LIB_VARS basename)
	IF (OSG_INCLUDE_DIR)
	    #MESSAGE(STATUS "Adjusting ${basename} ")

	    IF (OSG_${basename}_LIBRARY_DEBUG AND OSG_${basename}_LIBRARY_RELEASE)
			# if the generator supports configuration types then set
			# optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
			IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
				SET(OSG_${basename}_LIBRARY optimized ${OSG_${basename}_LIBRARY_RELEASE} debug ${OSG_${basename}_LIBRARY_DEBUG})
			ELSE(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
				# if there are no configuration types and CMAKE_BUILD_TYPE has no value
				# then just use the release libraries
				SET(OSG_${basename}_LIBRARY ${OSG_${basename}_LIBRARY_RELEASE} )
			ENDIF(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
			SET(OSG_${basename}_LIBRARIES optimized ${OSG_${basename}_LIBRARY_RELEASE} debug ${OSG_${basename}_LIBRARY_DEBUG})
	    ENDIF (OSG_${basename}_LIBRARY_DEBUG AND OSG_${basename}_LIBRARY_RELEASE)

	    # if only the release version was found, set the debug variable also to the release version
	    IF (OSG_${basename}_LIBRARY_RELEASE AND NOT OSG_${basename}_LIBRARY_DEBUG)
			SET(OSG_${basename}_LIBRARY_DEBUG ${OSG_${basename}_LIBRARY_RELEASE})
			SET(OSG_${basename}_LIBRARY       ${OSG_${basename}_LIBRARY_RELEASE})
			SET(OSG_${basename}_LIBRARIES     ${OSG_${basename}_LIBRARY_RELEASE})
	    ENDIF (OSG_${basename}_LIBRARY_RELEASE AND NOT OSG_${basename}_LIBRARY_DEBUG)

	    # if only the debug version was found, set the release variable also to the debug version
	    IF (OSG_${basename}_LIBRARY_DEBUG AND NOT OSG_${basename}_LIBRARY_RELEASE)
			SET(OSG_${basename}_LIBRARY_RELEASE ${OSG_${basename}_LIBRARY_DEBUG})
			SET(OSG_${basename}_LIBRARY         ${OSG_${basename}_LIBRARY_DEBUG})
			SET(OSG_${basename}_LIBRARIES       ${OSG_${basename}_LIBRARY_DEBUG})
	    ENDIF (OSG_${basename}_LIBRARY_DEBUG AND NOT OSG_${basename}_LIBRARY_RELEASE)
	    
	    IF (OSG_${basename}_LIBRARY)
			SET(OSG_${basename}_LIBRARY ${OSG_${basename}_LIBRARY} CACHE FILEPATH "The OSG ${basename} library")
			GET_FILENAME_COMPONENT(OSG_LIBRARY_DIRS "${OSG_${basename}_LIBRARY}" PATH)
			SET(OSG_LIBRARY_DIRS ${OSG_LIBRARY_DIRS} CACHE FILEPATH "OSG library directory")
			SET(OSG_${basename}_FOUND ON CACHE INTERNAL "Was the boost boost ${basename} library found")
	    ENDIF (OSG_${basename}_LIBRARY)

	ENDIF (OSG_INCLUDE_DIR)
	# Make variables changeble to the advanced user
	MARK_AS_ADVANCED(
		OSG_${basename}_LIBRARY
		OSG_${basename}_LIBRARY_RELEASE
	    OSG_${basename}_LIBRARY_DEBUG
	)
ENDMACRO (_OSG_ADJUST_LIB_VARS)

# **** Split library groups out into their components ****
#	SET(_osg_CombinedLib_MarineLibs vpmarine wavetools)
#	SET(_osg_CombinedLibs CoordSysLibs LoaderLibs MarineLibs ShaderLibs)
SET(_osg_CombinedLibs)

FOREACH(COMBINED_LIB ${_osg_CombinedLibs})			
	LIST(FIND OSG_FIND_COMPONENTS ${COMBINED_LIB} _combined_lib_requested)
	SET(_osg_Combined_Libs_${COMBINED_LIB} FALSE)
	
	IF (NOT ${_combined_lib_requested} EQUAL -1)
		# The CombinedLib is a requested component
		SET(_osg_Combined_Libs_${COMBINED_LIB} TRUE)
		
		LIST(REMOVE_ITEM OSG_FIND_COMPONENTS ${COMBINED_LIB})
		LIST(APPEND OSG_FIND_COMPONENTS ${_osg_CombinedLib_${COMBINED_LIB}})
	ENDIF(NOT ${_combined_lib_requested} EQUAL -1)
ENDFOREACH(COMBINED_LIB)

LIST(REMOVE_DUPLICATES OSG_FIND_COMPONENTS)
	
# **** The rest of it ****
SET( _osg_IN_CACHE TRUE)

IF(OSG_INCLUDE_DIR)
  FOREACH(COMPONENT ${OSG_FIND_COMPONENTS})
    STRING(TOUPPER ${COMPONENT} COMPONENT)
    IF(NOT OSG_${COMPONENT}_FOUND)
		SET( _osg_IN_CACHE FALSE)
    ENDIF(NOT OSG_${COMPONENT}_FOUND)
  ENDFOREACH(COMPONENT)
  SET(OSG_INCLUDE_DIRS "${OSG_INCLUDE_DIR}")

ELSE(OSG_INCLUDE_DIR)
	SET( _osg_IN_CACHE FALSE)
ENDIF(OSG_INCLUDE_DIR)

IF (_osg_IN_CACHE)
	# in cache already
	SET(OSG_FOUND TRUE)
	SET(OSG_LIBRARIES)
	FOREACH(COMPONENT ${OSG_FIND_COMPONENTS})
		STRING(TOUPPER ${COMPONENT} COMPONENT)
		_OSG_ADJUST_LIB_VARS( ${COMPONENT} )
		SET(OSG_LIBRARIES ${OSG_LIBRARIES} ${OSG_${COMPONENT}_LIBRARY})
	ENDFOREACH(COMPONENT)
ELSE (_osg_IN_CACHE)
	SET(_osg_SEARCH_PATH
		$ENV{OSG_DIR}
		$ENV{OSGDIR}
		$ENV{OSG_ROOT}
		$ENV{OpenThreads_ROOT}
	    ~/Library/Frameworks
	    /Library/Frameworks
	    /usr/local
	    /usr
	    /sw # Fink
	    /opt/local # DarwinPorts
	    /opt/csw # Blastwave
	    /opt
	)
	FIND_PATH(OSG_INCLUDE_DIR osg/PositionAttitudeTransform 
		PATHS ${_osg_SEARCH_PATH}
		PATH_SUFFIXES include 
	)

	IF(OSG_INCLUDE_DIR)
		# Extract OSG_VERSION from vp.h
		
		SET(OSG_VERSION 0)
		SET(_osg_VERSION_MAJOR 0)
		SET(_osg_VERSION_MINOR 0)
		SET(_osg_VERSION_PATCH 0)
		
		FILE(READ "${OSG_INCLUDE_DIR}/osg/Version" _osg_VERSION_H_CONTENTS)
		IF ("${_osg_VERSION_H_CONTENTS}" MATCHES ".*#define OSG_VERSION_MAJOR [0-9]+.*")
			# Pre 2.6 version
			STRING(REGEX REPLACE ".*#define OSG_VERSION_MAJOR ([0-9]+).*" "\\1" _osg_VERSION_MAJOR "${_osg_VERSION_H_CONTENTS}")
			STRING(REGEX REPLACE ".*#define OSG_VERSION_MINOR ([0-9]+).*" "\\1" _osg_VERSION_MINOR "${_osg_VERSION_H_CONTENTS}")
			STRING(REGEX REPLACE ".*#define OSG_VERSION_PATCH ([0-9]+).*" "\\1" _osg_VERSION_PATCH "${_osg_VERSION_H_CONTENTS}")			
		ELSEIF ("${_osg_VERSION_H_CONTENTS}" MATCHES ".*#define OPENSCENEGRAPH_MAJOR_VERSION[\\t ]+[0-9]+.*")
			# Post 2.6 version
			STRING(REGEX REPLACE ".*#define OPENSCENEGRAPH_MAJOR_VERSION[\\t ]+([0-9]+).*" "\\1" _osg_VERSION_MAJOR "${_osg_VERSION_H_CONTENTS}")
			STRING(REGEX REPLACE ".*#define OPENSCENEGRAPH_MINOR_VERSION[\\t ]+([0-9]+).*" "\\1" _osg_VERSION_MINOR "${_osg_VERSION_H_CONTENTS}")
			STRING(REGEX REPLACE ".*#define OPENSCENEGRAPH_PATCH_VERSION[\\t ]+([0-9]+).*" "\\1" _osg_VERSION_PATCH "${_osg_VERSION_H_CONTENTS}")
			STRING(REGEX REPLACE ".*#define OPENSCENEGRAPH_SOVERSION[\\t ]+([0-9]+).*" "\\1" _osg_VERSION_SO "${_osg_VERSION_H_CONTENTS}")
		ENDIF("${_osg_VERSION_H_CONTENTS}" MATCHES ".*#define OSG_VERSION_MAJOR [0-9]+.*")

		SET(OSG_MAJOR_VERSION "${_osg_VERSION_MAJOR}" CACHE INTERNAL "")
		SET(OSG_MINOR_VERSION "${_osg_VERSION_MINOR}" CACHE INTERNAL "")
		SET(OSG_PATCH_VERSION "${_osg_VERSION_PATCH}" CACHE INTERNAL "")
		SET(OSG_SO_VERSION "${_osg_VERSION_SO}" CACHE INTERNAL "")
		SET(OSG_VERSION "${_osg_VERSION_MAJOR}.${_osg_VERSION_MINOR}.${_osg_VERSION_PATCH}" CACHE INTERNAL "The version number for OSG libraries")

	ENDIF(OSG_INCLUDE_DIR)

	# ------------------------------------------------------------------------
	#  Begin finding OSG libraries
	# ------------------------------------------------------------------------
	SET(_osg_LIB_VERSION "${_osg_VERSION_MAJOR}_${_osg_VERSION_MINOR}")
	SET(_osg_LIBRARIES_SEARCH_DIRS ${_osg_SEARCH_PATH})

	# Support preference of static libs by adjusting CMAKE_FIND_LIBRARY_SUFFIXES
	SET(_osg_DEBUG_TAG "")
	SET(_osg_LIB_PREFIX "")
	IF(WIN32)
		SET(_osg_DEBUG_TAG "d")
	ELSE(WIN32)
		SET(_osg_LIB_PREFIX "lib")
		SET(_osg_DEBUG_TAG "d")
	ENDIF(WIN32)
	
	FOREACH(COMPONENT ${OSG_FIND_COMPONENTS})
		STRING(TOUPPER ${COMPONENT} UPPERCOMPONENT)
		SET( OSG_${UPPERCOMPONENT}_LIBRARY "OSG_${UPPERCOMPONENT}_LIBRARY-NOTFOUND" )
		SET( OSG_${UPPERCOMPONENT}_LIBRARY_RELEASE "OSG_${UPPERCOMPONENT}_LIBRARY_RELEASE-NOTFOUND" )
		SET( OSG_${UPPERCOMPONENT}_LIBRARY_DEBUG "OSG_${UPPERCOMPONENT}_LIBRARY_DEBUG-NOTFOUND")

		FIND_LIBRARY(OSG_${UPPERCOMPONENT}_LIBRARY_RELEASE
			NAMES 
				${_osg_LIB_PREFIX}${COMPONENT}${_osg_LIB_VERSION}
				${_osg_LIB_PREFIX}${COMPONENT}
				${COMPONENT}${_osg_LIB_VERSION}
				${COMPONENT}
			PATHS  ${_osg_LIBRARIES_SEARCH_DIRS}
			PATH_SUFFIXES lib
		)

		FIND_LIBRARY(OSG_${UPPERCOMPONENT}_LIBRARY_DEBUG
			NAMES  
				${_osg_LIB_PREFIX}${COMPONENT}${_osg_LIB_VERSION}${_osg_DEBUG_TAG}
				${_osg_LIB_PREFIX}${COMPONENT}${_osg_DEBUG_TAG}
				${COMPONENT}${_osg_LIB_VERSION}${_osg_DEBUG_TAG}
				${COMPONENT}${_osg_DEBUG_TAG}
			PATHS  ${_osg_LIBRARIES_SEARCH_DIRS}
			PATH_SUFFIXES lib
		)

		_OSG_ADJUST_LIB_VARS(${UPPERCOMPONENT})
		
	ENDFOREACH(COMPONENT)	
	
	SET(OSG_INCLUDE_DIRS "${OSG_INCLUDE_DIR}")

	SET(OSG_FOUND FALSE)
	IF(OSG_INCLUDE_DIR)
		SET( OSG_FOUND TRUE )
		set(_osg_CHECKED_COMPONENT FALSE)
		FOREACH(COMPONENT ${OSG_FIND_COMPONENTS})
			STRING(TOUPPER ${COMPONENT} COMPONENT)
			set(_osg_CHECKED_COMPONENT TRUE)
			IF(NOT OSG_${COMPONENT}_FOUND)
				SET( OSG_FOUND FALSE)
			ENDIF(NOT OSG_${COMPONENT}_FOUND)
		ENDFOREACH(COMPONENT)
	ELSE(OSG_INCLUDE_DIR)
		SET(OSG_FOUND FALSE)
	ENDIF(OSG_INCLUDE_DIR)

	IF (OSG_FOUND)
		# Combine the  components of a CombinedLib back into a set of  LIBRARY references
		FOREACH(COMBINED_LIB ${_osg_CombinedLibs})			
			IF(_osg_Combined_Libs_${COMBINED_LIB})
				# This combined lib was created
				STRING(TOUPPER ${COMBINED_LIB} UPPERCOMBINED_LIB)
				SET(OSG_${UPPERCOMBINED_LIB}_LIBRARY)
				SET(OSG_${UPPERCOMBINED_LIB}_LIBRARY_DEBUG)
				SET(OSG_${UPPERCOMBINED_LIB}_LIBRARY_RELEASE)
				FOREACH(BASE_LIB ${_osg_CombinedLib_${COMBINED_LIB}})
					STRING(TOUPPER ${BASE_LIB} UPPERBASE_LIB)
				
					LIST(APPEND OSG_${UPPERCOMBINED_LIB}_LIBRARY ${OSG_${UPPERBASE_LIB}_LIBRARY})
					LIST(APPEND OSG_${UPPERCOMBINED_LIB}_LIBRARY_DEBUG ${OSG_${UPPERBASE_LIB}_LIBRARY_DEBUG})
					LIST(APPEND OSG_${UPPERCOMBINED_LIB}_LIBRARY_RELEASE ${OSG_${UPPERBASE_LIB}_LIBRARY_RELEASE})
				ENDFOREACH(BASE_LIB)
				SET(OSG_${UPPERCOMBINED_LIB}_LIBRARY_DEBUG ${OSG_${UPPERCOMBINED_LIB}_LIBRARY_DEBUG} CACHE FILE_PATH "")
				SET(OSG_${UPPERCOMBINED_LIB}_LIBRARY_RELEASE ${OSG_${UPPERCOMBINED_LIB}_LIBRARY_RELEASE} CACHE FILE_PATH "")
				SET(OSG_${UPPERCOMBINED_LIB}_LIBRARY ${OSG_${UPPERCOMBINED_LIB}_LIBRARY} CACHE FILEPATH "The OSG ${UPPERCOMBINED_LIB} library")
				SET(OSG_${UPPERCOMBINED_LIB}_FOUND ON CACHE INTERNAL "Was the boost boost ${UPPERCOMBINED_LIB} library found")
			ENDIF(_osg_Combined_Libs_${COMBINED_LIB})
		ENDFOREACH(COMBINED_LIB)		

		IF (NOT OSG_FIND_QUIETLY)
			MESSAGE(STATUS "OSG version: ${OSG_VERSION}")
			MESSAGE(STATUS "Found the following OSG libraries:")
		ENDIF(NOT OSG_FIND_QUIETLY)
		
		FOREACH ( COMPONENT  ${OSG_FIND_COMPONENTS} )
			STRING( TOUPPER ${COMPONENT} UPPERCOMPONENT )
			IF ( OSG_${UPPERCOMPONENT}_FOUND )
			  IF (NOT OSG_FIND_QUIETLY)
				MESSAGE (STATUS "  ${COMPONENT}")
			  ENDIF(NOT OSG_FIND_QUIETLY)
			  SET(OSG_LIBRARIES ${OSG_LIBRARIES} ${OSG_${UPPERCOMPONENT}_LIBRARY})
			ENDIF ( OSG_${UPPERCOMPONENT}_FOUND )
		ENDFOREACH(COMPONENT)
	ELSE (OSG_FOUND)
	
		IF (OSG_FIND_REQUIRED)
			MESSAGE(FATAL_ERROR "Couldn't find the OSG libraries and/or include directory. Please install the OSG libraries AND development packages.")
		ENDIF(OSG_FIND_REQUIRED)
	ENDIF(OSG_FOUND)
	
	MARK_AS_ADVANCED(OSG_INCLUDE_DIR)

ENDIF(_osg_IN_CACHE)
