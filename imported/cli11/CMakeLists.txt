### Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved. ###
project(CLI11 VERSION 2.4.1)
add_library(cli11 INTERFACE)
target_include_directories(cli11 INTERFACE ${CMAKE_CURRENT_LIST_DIR})