### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
project(zstd VERSION 1.4.5)
add_library(zstd STATIC src/zstd.c)
target_include_directories(zstd PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)

if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(zstd PRIVATE Threads::Threads)
endif()