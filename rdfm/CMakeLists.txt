### Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved. ###
add_executable(rdfm)
target_sources(rdfm PRIVATE
    src/rdfm.cpp)

target_link_libraries(rdfm PRIVATE rdf cli11 json)
set_target_properties(rdfm PROPERTIES CXX_STANDARD 17)

if(RDF_BUILD_INSTALL)
    install(TARGETS rdfm DESTINATION bin)
endif()
