project(debugger_subprocess)

set(SOURCES
    main.cpp
)

add_executable(debugger_subprocess ${SOURCES})

target_link_libraries(debugger_subprocess ${Boost_LIBRARIES} -lrt)

target_compile_features(debugger_subprocess PRIVATE cxx_std_17)

set(CMAKE_CXX_FLAGS -pthread)