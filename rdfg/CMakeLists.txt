### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
add_executable(rdfg)
target_sources(rdfg PRIVATE
    src/rdfg.cpp)

target_link_libraries(rdfg PRIVATE rdf cli11 json)
set_target_properties(rdfg PROPERTIES CXX_STANDARD 17)

if(RDF_BUILD_INSTALL)
    install(TARGETS rdfg DESTINATION bin)
endif()
