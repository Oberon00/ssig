# Part of ssig -- Copyright (c) Christian Neumüller 2012--2013
# This file is subject to the terms of the BSD 2-Clause License.
# See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

set(Boost_USE_STATIC_LIBS    ON)
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_FIND_COMPONENTS unit_test_framework signals system chrono timer)
find_package(Boost REQUIRED unit_test_framework signals system chrono timer)

set (COMP_DEFINITIONS "BOOST_ALL_NO_LIB")

if (MSVC)
    add_definitions(
        /W4     # warning level 4
        /MP)    # multicore compilation
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
endif ()

include_directories("../include" ${Boost_INCLUDE_DIRS})

add_executable(ssig_test "test.cpp")
target_link_libraries(ssig_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
set_target_properties(ssig_test PROPERTIES
    COMPILE_DEFINITIONS ${COMP_DEFINITIONS})

add_test(ssig_test_run ssig_test)

add_executable(ssig_benchmark "benchmark.cpp")
target_link_libraries(ssig_benchmark
    ${Boost_SIGNALS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_TIMER_LIBRARY})
set_target_properties(ssig_benchmark PROPERTIES
    COMPILE_DEFINITIONS ${COMP_DEFINITIONS})
