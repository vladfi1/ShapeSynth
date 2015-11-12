#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)


FIND_PATH(OPENMESH_INCLUDE_DIR OpenMesh/Core/Mesh/TriMeshT.hh ${CMAKE_SOURCE_DIR}/../3rdparty/OpenMesh/include) 
FIND_LIBRARY(OPENMESH_CORE_LIBRARY OpenMeshCored ${CMAKE_SOURCE_DIR}/../3rdparty/OpenMesh/lib)
FIND_LIBRARY(OPENMESH_TOOLS_LIBRARY OpenMeshToolsd ${CMAKE_SOURCE_DIR}/../3rdparty/OpenMesh/lib)


SET( OPENMESH_LIBRARIES ${OPENMESH_CORE_LIBRARY} ${OPENMESH_TOOLS_LIBRARY} )
SET( OPENMESH_INCLUDE_DIRS ${OPENMESH_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set OPENMESH_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(OpenMesh DEFAULT_MSG OPENMESH_CORE_LIBRARY OPENMESH_TOOLS_LIBRARY OPENMESH_INCLUDE_DIR)

MARK_AS_ADVANCED( OPENMESH_INCLUDE_DIR OPENMESH_CORE_LIBRARY OPENMESH_TOOLS_LIBRARY )
