project(initialization_file_parser)

set(HEADERS
    include/initialization_file_parser.h
)

set(SOURCES
    src/initialization_file_parser.cpp
)

add_library(initialization_file_parser ${HEADERS} ${SOURCES})

target_include_directories(initialization_file_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(initialization_file_parser PRIVATE cxx_std_17)