# This module defines
#  QCodeEditor_FOUND, if false, do not try to link
#  QCodeEditor_LIBRARIES, libraries to link against
#  QCodeEditor_INCLUDE_DIR, where to find headers

FIND_PATH(QCodeEditor_INCLUDE_DIR KGL/Widgets/QCodeEditor.hpp
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(QCodeEditor_LIB_R
  NAMES QCodeEditor
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(QCodeEditor_LIB_D
  NAMES QCodeEditord
  PATH_SUFFIXES
  lib
)

SET(QCodeEditor_LIBRARIES
	debug ${QCodeEditor_LIB_D}
	optimized ${QCodeEditor_LIB_R}
)

IF(QCodeEditor_LIBRARIES AND QCodeEditor_INCLUDE_DIR)
  SET(QCodeEditor_FOUND "YES")
ENDIF()