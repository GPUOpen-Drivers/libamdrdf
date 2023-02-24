### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
cmake_minimum_required(VERSION 3.13..3.21)
project(rdf LANGUAGES CXX C)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(RDF_IS_TOP_LEVEL ON)
else()
    set(RDF_IS_TOP_LEVEL OFF)
endif()

option(RDF_BUILD_TESTS         "Build the tests"  ${RDF_IS_TOP_LEVEL})
option(RDF_BUILD_TOOLS         "Build RDF tools"  ${RDF_IS_TOP_LEVEL})
option(RDF_BUILD_INSTALL       "Install RDF libs" ${RDF_IS_TOP_LEVEL})
option(RDF_ENABLE_CXX_BINDINGS "Allow usage of C++ interface for RDF library" ON)
option(RDF_STATIC              "Build RDF as a static library" OFF)

if(RDF_BUILD_TESTS)
    enable_testing()
endif()

# Set the C++ standard for all libraries/executables in RDF
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Don't modify these variables for projects that consume this libary using add_subdirectory
if (RDF_IS_TOP_LEVEL)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    if("${isSystemDir}" STREQUAL "-1")
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    endif()

    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

add_subdirectory(imported)
add_subdirectory(rdf)
if(RDF_BUILD_TOOLS)
    add_subdirectory(rdfi)
    add_subdirectory(rdfg)
    add_subdirectory(rdfm)
endif()
