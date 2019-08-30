# This module defines
#  Qwt_FOUND, if false, do not try to link
#  Qwt_LIBRARIES, libraries to link against
#  Qwt_INCLUDE_DIR, where to find headers

FIND_PATH(Qwt_INCLUDE_DIR qwt/qwt.h
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(Qwt_LIB_R
  NAMES qwt
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(Qwt_LIB_D
  NAMES qwtd
  PATH_SUFFIXES
  lib
)

SET(Qwt_LIBRARIES
	debug ${Qwt_LIB_D}
	optimized ${Qwt_LIB_R}
)

IF(Qwt_LIBRARIES AND Qwt_INCLUDE_DIR)
  SET(Qwt_FOUND "YES")
ENDIF()