### Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved. ###
project(json VERSION 3.11.3)
add_library(json INTERFACE)
target_include_directories(json INTERFACE ${CMAKE_CURRENT_LIST_DIR})