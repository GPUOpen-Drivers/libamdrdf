### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
add_executable(rdfi)
target_sources(rdfi PRIVATE
    src/rdfi.cpp)

target_link_libraries(rdfi PRIVATE rdf cli11 json)
set_target_properties(rdfi PROPERTIES CXX_STANDARD 17)

if(RDF_BUILD_INSTALL)
    install(TARGETS rdfi DESTINATION bin)
endif()
