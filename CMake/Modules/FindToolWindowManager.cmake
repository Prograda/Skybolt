# This module defines
#  ToolWindowManager_FOUND, if false, do not try to link
#  ToolWindowManager_LIBRARIES, libraries to link against
#  ToolWindowManager_INCLUDE_DIR, where to find headers

FIND_PATH(ToolWindowManager_INCLUDE_DIR qtoolwindowmanager.h
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(ToolWindowManager_LIBRARY_RELEASE
  NAMES qtoolwindowmanager
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(ToolWindowManager_LIBRARY_DEBUG
  NAMES qtoolwindowmanagerd
  PATH_SUFFIXES
  lib
)

select_library_configurations(ToolWindowManager)

IF(ToolWindowManager_LIBRARIES AND ToolWindowManager_INCLUDE_DIR)
  SET(ToolWindowManager_FOUND "YES")
ENDIF()