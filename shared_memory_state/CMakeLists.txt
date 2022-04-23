project(shared_memory_state)

set(HEADERS
    include/shared_memory_state.h
)

set(SOURCES
    src/shared_memory_state.cpp
)

add_library(shared_memory_state ${HEADERS} ${SOURCES})

target_include_directories(shared_memory_state PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(shared_memory_state PRIVATE cxx_std_17)