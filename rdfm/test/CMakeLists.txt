### Copyright (c) 2021-2023 Advanced Micro Devices, Inc. All rights reserved. ###
add_test(NAME Test.RDFM.MergeEmptyChunkEmptyHeader
         COMMAND rdfm merge
            "${CMAKE_CURRENT_LIST_DIR}/data/empty-chunk.rdf"
            "${CMAKE_CURRENT_LIST_DIR}/data/empty-header.rdf"
            merged-empty-chunk-empty-header.rdf)
