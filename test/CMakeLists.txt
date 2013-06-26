# Part of ssig -- Copyright (c) Christian Neumüller 2012--2013
# This file is subject to the terms of the BSD 2-Clause License.
# See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

set(Boost_USE_STATIC_LIBS    ON)
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_FIND_COMPONENTS unit_test_framework)
find_package(Boost REQUIRED unit_test_framework)


set (COMP_DEFINITIONS "BOOST_ALL_NO_LIB")

if (MSVC)
    add_definitions(
        /W4     # warning level 4
        /MP)    # multicore compilation
else ()
    # see http://stackoverflow.com/a/11004675/2128694
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                        OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
    endif ()
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else ()
        message(FATAL_ERROR "Your C++ compiler does not support C++11.")
    endif ()
endif ()

add_executable(ssig_test "test.cpp")
include_directories("../include" ${Boost_INCLUDE_DIRS})
target_link_libraries(ssig_test ${Boost_LIBRARIES})
set_target_properties(ssig_test PROPERTIES
    COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB")

add_test(ssig_test_run ssig_test)
