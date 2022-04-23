project(debugger)

set(HEADERS
    include/debugger.h
)

set(SOURCES
    src/debugger.cpp
)

add_library(debugger ${HEADERS} ${SOURCES})

target_include_directories(debugger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(debugger ${Boost_LIBRARIES} -lrt)


target_compile_features(debugger PRIVATE cxx_std_17)

