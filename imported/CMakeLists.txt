### Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. ###
add_subdirectory(zstd)

# This allows clients of RDF to have their own copy of catch2/cli11/json, and
# still build RDF "in-tree" when imported via a submodule. This is just
# convenience, when building in-tree there is no guarantee that tools & tests
# will work
if(RDF_BUILD_TESTS)
	if(NOT TARGET catch2)
		add_subdirectory(catch2)
	endif()
endif()

if(RDF_BUILD_TOOLS)
	if(NOT TARGET cli11)
		add_subdirectory(cli11)
	endif()
	if(NOT TARGET json)
		add_subdirectory(json)
	endif()
endif()
