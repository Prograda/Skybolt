# This module defines
#  muFFT_FOUND, if false, do not try to link
#  muFFT_LIBRARIES, libraries to link against
#  muFFT_INCLUDE_DIR, where to find headers

FIND_PATH(muFFT_INCLUDE_DIR muFFT/fft.h
  PATH_SUFFIXES
  include
)

FIND_LIBRARY(muFFT_CORE_LIBRARY
  NAMES muFFT
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(muFFT_SSE_LIBRARY
  NAMES muFFT-sse
  PATH_SUFFIXES
  lib
)

FIND_LIBRARY(muFFT_SSE3_LIBRARY
  NAMES muFFT-sse3
  PATH_SUFFIXES
  lib
)


FIND_LIBRARY(muFFT_AVX_LIBRARY
  NAMES muFFT-avx
  PATH_SUFFIXES
  lib
)

set(muFFT_LIBRARIES
	${muFFT_CORE_LIBRARY}
	${muFFT_SSE_LIBRARY}
	${muFFT_SSE3_LIBRARY}
	${muFFT_AVX_LIBRARY}
)

IF(muFFT_LIBRARIES AND muFFT_INCLUDE_DIR)
  SET(muFFT_FOUND "YES")
ENDIF()