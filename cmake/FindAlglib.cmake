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

FIND_PATH(ALGLIB_INCLUDE_DIR linalg.h /usr/local/include/alglib /usr/include/)
FIND_LIBRARY(ALGLIB_LIBRARY alglib /usr/local/lib/alglib /usr/lib/x86_64-linux-gnu/)

SET(ALGLIB_LIBRARIES ${ALGLIB_LIBRARY})
SET(ALGLIB_INCLUDE_DIRS ${ALGLIB_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ALGLIB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Alglib DEFAULT_MSG ALGLIB_LIBRARY ALGLIB_INCLUDE_DIR)

MARK_AS_ADVANCED(ALGLIB_INCLUDE_DIR ALGLIB_LIBRARY)
