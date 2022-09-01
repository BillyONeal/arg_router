### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

add_library(arg_router INTERFACE ${HEADERS} ${FOR_IDE})
target_compile_features(arg_router INTERFACE cxx_std_17)
target_include_directories(arg_router
    INTERFACE "${CMAKE_SOURCE_DIR}/include"
)

if(NOT INSTALLATION_ONLY)
    add_dependencies(arg_router clangformat)
endif()
