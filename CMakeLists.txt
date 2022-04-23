cmake_minimum_required(VERSION 3.19)
project(cmake_debugger)

set(CMAKE_CXX_STANDARD 14)


add_subdirectory(debugger)
add_subdirectory(initialization_file_parser)
add_subdirectory(shared_memory_state)
add_subdirectory(breakpoint_subprocess)

add_executable(cmake_debugger main.cpp)

file(COPY config/config.ini DESTINATION config)
file(COPY cmake_utils/debugger_utils.cmake DESTINATION cmake_utils)

target_link_libraries(debugger ${Boost_LIBRARIES} -lrt)

target_link_libraries(${PROJECT_NAME} PUBLIC debugger)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/debugger/include)

target_link_libraries(${PROJECT_NAME} PUBLIC initialization_file_parser)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/initialization_file_parser/include)

target_link_libraries(${PROJECT_NAME} PUBLIC shared_memory_state)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/shared_memory_state/include)

target_compile_features(cmake_debugger PRIVATE cxx_std_17)

set(CMAKE_CXX_FLAGS -pthread)
