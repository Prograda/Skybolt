# - Try to find the Bullet physics engine
#
#  This module defines the following variables
#
#  BULLET_FOUND - Was bullet found
#  BULLET_INCLUDE_DIRS - the Bullet include directories
#  BULLET_LIBRARIES - Link to this, by default it includes
#                     all bullet components (Dynamics,
#                     Collision, LinearMath, & SoftBody)
#
#  This module accepts the following variables
#
#  BULLET_ROOT - Can be set to bullet install path or Windows build path
#

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (NOT DEFINED BULLET_ROOT AND DEFINED ENV{BULLET_ROOT})
	set(BULLET_ROOT $ENV{BULLET_ROOT})
endif()

macro(_FIND_BULLET_LIBRARY _var)
  find_library(${_var}
     NAMES 
        ${ARGN}
     HINTS
        ${BULLET_ROOT}/lib
        ${BULLET_ROOT}/out/release8/libs
     PATH_SUFFIXES Release RelWithDebInfo MinSizeDeb
  )
  mark_as_advanced(${_var})
endmacro()

macro(_FIND_BULLET_LIBRARY_DBG _var)
  find_library(${_var}
     NAMES 
        ${ARGN}
     HINTS
        ${BULLET_ROOT}/lib
        ${BULLET_ROOT}/out/debug8/libs
     PATH_SUFFIXES Debug
  )
  mark_as_advanced(${_var})
endmacro()

macro(_BULLET_APPEND_LIBRARIES _list _release)
   set(_debug ${_release}_DEBUG)
   if(${_debug})
      set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
   else()
      set(${_list} ${${_list}} ${${_release}})
   endif()
endmacro()

find_path(BULLET_INCLUDE_DIR NAMES btBulletCollisionCommon.h
  HINTS
    ${BULLET_ROOT}/include
    ${BULLET_ROOT}/src
  PATH_SUFFIXES bullet
)

find_path(CONVEX_INCLUDE_DIR NAMES ConvexBuilder.h
  HINTS
    ${BULLET_ROOT}/Extras/ConvexDecomposition
)

find_path(BULLET_WORLD_IMPORTER_INCLUDE_DIR NAMES btBulletWorldImporter.h
  HINTS
    ${BULLET_ROOT}/Extras/Serialize/BulletWorldImporter
)

find_path(BULLET_FILE_LOADER_INCLUDE_DIR NAMES btBulletFile.h
  HINTS
    ${BULLET_ROOT}/Extras/Serialize/BulletFileLoader
)

# Find the libraries

_FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIBRARY        BulletDynamics)
_FIND_BULLET_LIBRARY_DBG(BULLET_DYNAMICS_LIBRARY_DEBUG  BulletDynamics_Debug BulletDynamics_d BulletDynamics)
_FIND_BULLET_LIBRARY(BULLET_COLLISION_LIBRARY       BulletCollision)
_FIND_BULLET_LIBRARY_DBG(BULLET_COLLISION_LIBRARY_DEBUG BulletCollision_Debug BulletCollision_d BulletCollision)
_FIND_BULLET_LIBRARY(BULLET_MATH_LIBRARY            BulletMath LinearMath)
_FIND_BULLET_LIBRARY_DBG(BULLET_MATH_LIBRARY_DEBUG      BulletMath_Debug BulletMath_Debug LinearMath_Debug BulletMath LinearMath)
_FIND_BULLET_LIBRARY(CONVEX_DECOMPOSITION_LIBRARY        ConvexDecomposition)
_FIND_BULLET_LIBRARY_DBG(CONVEX_DECOMPOSITION_LIBRARY_DEBUG  ConvexDecomposition_Debug ConvexDecomposition_d ConvexDecomposition)
_FIND_BULLET_LIBRARY(BULLET_WORLD_IMPORTER_LIBRARY        BulletWorldImporter)
_FIND_BULLET_LIBRARY_DBG(BULLET_WORLD_IMPORTER_LIBRARY_DEBUG  BulletWorldImporter_Debug BulletWorldImporter_d BulletWorldImporter)
_FIND_BULLET_LIBRARY(BULLET_FILE_LOADER_LIBRARY        BulletFileLoader)
_FIND_BULLET_LIBRARY_DBG(BULLET_FILE_LOADER_LIBRARY_DEBUG  BulletFileLoader_Debug BulletFileLoader_d BulletFileLoader)

# handle the QUIETLY and REQUIRED arguments and set BULLET_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BULLET DEFAULT_MSG
    BULLET_DYNAMICS_LIBRARY BULLET_COLLISION_LIBRARY BULLET_MATH_LIBRARY
    CONVEX_DECOMPOSITION_LIBRARY BULLET_WORLD_IMPORTER_LIBRARY
	BULLET_FILE_LOADER_LIBRARY BULLET_INCLUDE_DIR)

set(BULLET_INCLUDE_DIRS ${BULLET_INCLUDE_DIR} ${CONVEX_INCLUDE_DIR} ${BULLET_WORLD_IMPORTER_INCLUDE_DIR})
if(BULLET_FOUND)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_DYNAMICS_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_COLLISION_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_MATH_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES CONVEX_DECOMPOSITION_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_WORLD_IMPORTER_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_FILE_LOADER_LIBRARY)
endif()
