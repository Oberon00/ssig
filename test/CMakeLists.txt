# Part of ssig -- Copyright (c) Christian Neumüller 2012--2013
# This file is subject to the terms of the BSD 2-Clause License.
# See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

set(Boost_USE_STATIC_LIBS    ON)
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_FIND_COMPONENTS unit_test_framework)
find_package(Boost REQUIRED unit_test_framework)

add_executable(ssig_test "test.cpp")
include_directories("../include" ${Boost_INCLUDE_DIRS})
target_link_libraries(ssig_test ${Boost_LIBRARIES})
set_target_properties(ssig_test PROPERTIES
    COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB")

add_test(ssig_test_run ssig_test)
