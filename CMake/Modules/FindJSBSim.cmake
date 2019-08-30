# This module defines
#  JSBSIM_FOUND, if false, do not try to link
#  JSBSIM_LIBRARIES, libraries to link against
#  JSBSIM_INCLUDE_DIR, where to find headers

FIND_PATH(JSBSIM_INCLUDE_DIR jsbsim/FGFDMExec.h
  PATHS
  $ENV{JSBSIM_ROOT}
  PATH_SUFFIXES include
)

FIND_LIBRARY(JSBSIM_LIB_R
  NAMES JSBSim
  PATHS
  $ENV{JSBSIM_ROOT}/lib
  PATH_SUFFIXES lib
)

SET(JSBSIM_LIBRARIES
	optimized ${JSBSIM_LIB_R}
)

IF(JSBSIM_LIBRARIES AND JSBSIM_INCLUDE_DIR)
  SET(JSBSIM_FOUND "YES")
ENDIF()