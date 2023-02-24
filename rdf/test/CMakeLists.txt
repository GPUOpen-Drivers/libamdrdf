### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
add_executable(rdf.Test)
target_sources(rdf.Test PRIVATE
    inc/test_rdf.h
    src/IO_test.cpp
    src/RDF_test.cpp
    src/test_file.c
    src/main.cpp
)

target_link_libraries(rdf.Test PRIVATE rdf catch2)
target_include_directories(rdf.Test PRIVATE inc)
add_test(NAME rdf.Test COMMAND rdf.Test)

if(RDF_BUILD_INSTALL)
    install(TARGETS rdf.Test DESTINATION bin)
endif()