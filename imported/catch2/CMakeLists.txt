### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
project(Catch2)
add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE ${CMAKE_CURRENT_LIST_DIR})