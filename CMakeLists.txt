# Part of ssig -- Copyright (c) Christian Neumüller 2012--2013
# This file is subject to the terms of the BSD 2-Clause License.
# See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

cmake_minimum_required(VERSION 2.8)
project(ssig)

if (NOT SSIG_SKIP_TESTS)
    enable_testing()
    add_subdirectory("test")
endif()

install(DIRECTORY "include/" DESTINATION "include")

if(WIN32 AND NOT CYGWIN)
  set(INSTALL_CMAKE_DIR CMake)
else()
  set(INSTALL_CMAKE_DIR lib/CMake/ssig)
endif()
INSTALL(FILES "cmake/Modules/Findssig.cmake" DESTINATION ${INSTALL_CMAKE_DIR})
