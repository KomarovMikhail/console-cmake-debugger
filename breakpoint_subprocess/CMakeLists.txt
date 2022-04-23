project(breakpoint_subprocess)

set(SOURCES
    main.cpp
)

add_executable(breakpoint_subprocess ${SOURCES})

target_link_libraries(breakpoint_subprocess shared_memory_state)
target_link_libraries(breakpoint_subprocess ${Boost_LIBRARIES} -lrt)

target_compile_features(breakpoint_subprocess PRIVATE cxx_std_17)

set(CMAKE_CXX_FLAGS -pthread)