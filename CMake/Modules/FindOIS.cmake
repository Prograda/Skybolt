# This module defines
#  OIS_FOUND, if false, do not try to link
#  OIS_LIBRARY, the library to link against
#  OIS_INCLUDE_DIR, where to find headers


FIND_PATH(OIS_INCLUDE_DIR OIS/OIS.h
  PATHS
  $ENV{OGRE_DEPENDENCIES_DIR}
  PATH_SUFFIXES
	include
)

FIND_LIBRARY(OIS_LIBRARY_R
  NAMES OIS
  PATHS
  $ENV{OGRE_DEPENDENCIES_DIR}/lib/Release
)

FIND_LIBRARY(OIS_LIBRARY_D
  NAMES OISd OIS
  PATHS
  $ENV{OGRE_DEPENDENCIES_DIR}/lib/Debug
)

SET(OIS_LIBRARIES optimized ${OIS_LIBRARY_R} debug ${OIS_LIBRARY_D})

IF(OIS_LIBRARY AND OIS_INCLUDE_DIR)
  SET(OIS_FOUND "YES")
ENDIF()