# This module defines
#  cigicl_FOUND, if false, do not try to link
#  cigicl_LIBRARIES, libraries to link against
#  cigicl_INCLUDE_DIR, where to find headers

FIND_PATH(cigicl_INCLUDE_DIR cigicl/CigiTypes.h
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(cigicl_LIB_R
  NAMES ccl_dll
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(cigicl_LIB_D
  NAMES ccl_dllD
  PATH_SUFFIXES
  lib
)

SET(cigicl_LIBRARIES
	debug ${cigicl_LIB_D}
	optimized ${cigicl_LIB_R}
)

IF(cigicl_LIBRARIES AND cigicl_INCLUDE_DIR)
  SET(cigicl_FOUND "YES")
ENDIF()