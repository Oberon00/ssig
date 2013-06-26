# Part of ssig -- Copyright (c) Christian Neumüller 2012--2013
# This file is subject to the terms of the BSD 2-Clause License.
# See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

# Locate ssig library (i.e. header files)

FIND_PATH(SSIG_INCLUDE_DIRS ssig.hpp
  HINTS $ENV{SSIG_DIR}
  PATH_SUFFIXES include/sssig include
)

INCLUDE(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
# handle the QUIETLY and REQUIRED arguments and set SSIG_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ssig REQUIRED_VARS SSIG_INCLUDE_DIRS)

MARK_AS_ADVANCED(SSIG_INCLUDE_DIRS)
