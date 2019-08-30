# This module defines
#  READOSM_FOUND, if false, do not try to link
#  READOSM_LIBRARIES, libraries to link against
#  READOSM_INCLUDE_DIR, where to find headers

FIND_PATH(READOSM_INCLUDE_DIR readosm/readosm.h
  PATHS
  $ENV{READOSM_ROOT}
  PATH_SUFFIXES include
)

FIND_LIBRARY(READOSM_LIB_R
  NAMES readosm
  PATHS
  $ENV{READOSM_ROOT}/lib
  PATH_SUFFIXES lib
)

FIND_LIBRARY(READOSM_LIB_D
  NAMES readosmd
  PATHS
  $ENV{READOSM_ROOT}/lib
  PATH_SUFFIXES lib
)

SET(READOSM_LIBRARIES
	debug ${READOSM_LIB_D}
	optimized ${READOSM_LIB_R}
)

IF(READOSM_LIBRARIES AND READOSM_INCLUDE_DIR)
  SET(READOSM_FOUND "YES")
ENDIF()