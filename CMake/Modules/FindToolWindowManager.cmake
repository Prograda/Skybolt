# This module defines
#  ToolWindowManager_FOUND, if false, do not try to link
#  ToolWindowManager_LIBRARIES, libraries to link against
#  ToolWindowManager_INCLUDE_DIR, where to find headers

FIND_PATH(ToolWindowManager_INCLUDE_DIR qtoolwindowmanager.h
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(ToolWindowManager_LIB_R
  NAMES qtoolwindowmanager
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(ToolWindowManager_LIB_D
  NAMES qtoolwindowmanagerd
  PATH_SUFFIXES
  lib
)

SET(ToolWindowManager_LIBRARIES
	debug ${ToolWindowManager_LIB_D}
	optimized ${ToolWindowManager_LIB_R}
)

IF(ToolWindowManager_LIBRARIES AND ToolWindowManager_INCLUDE_DIR)
  SET(ToolWindowManager_FOUND "YES")
ENDIF()