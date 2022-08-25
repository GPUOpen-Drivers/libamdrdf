### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
project(json VERSION 3.9.1)
add_library(json INTERFACE)
target_include_directories(json INTERFACE ${CMAKE_CURRENT_LIST_DIR})