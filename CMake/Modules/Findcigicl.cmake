# This module defines
#  cigicl_FOUND, if false, do not try to link
#  cigicl_LIBRARIES, libraries to link against
#  cigicl_INCLUDE_DIR, where to find headers

FIND_PATH(cigicl_INCLUDE_DIR cigicl/CigiTypes.h
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(cigicl_LIBRARY_RELEASE
  NAMES ccl_dll
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(cigicl_LIBRARY_DEBUG
  NAMES ccl_dllD
  PATH_SUFFIXES
  lib
)

select_library_configurations(cigicl)

IF(cigicl_LIBRARIES AND cigicl_INCLUDE_DIR)
  SET(cigicl_FOUND "YES")
ENDIF()