# This module defines
#  NodeEditor_FOUND, if false, do not try to link
#  NodeEditor_LIBRARIES, libraries to link against
#  NodeEditor_INCLUDE_DIR, where to find headers

FIND_PATH(NodeEditor_INCLUDE_DIR nodes/Node
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(NodeEditor_LIB_R
  NAMES nodes
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(NodeEditor_LIB_D
  NAMES nodesd
  PATH_SUFFIXES
  lib
)

SET(NodeEditor_LIBRARIES
	debug ${NodeEditor_LIB_D}
	optimized ${NodeEditor_LIB_R}
)

IF(NodeEditor_LIBRARIES AND NodeEditor_INCLUDE_DIR)
  SET(NodeEditor_FOUND "YES")
ENDIF()