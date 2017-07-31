cmake_minimum_required(VERSION 3.2)
project(CIRCULAR_BUFFER CXX)

set(header_files
	${PROJECT_SOURCE_DIR}/include/circular_buffer.hpp)

add_library(circular_buffer INTERFACE)

target_sources(circular_buffer INTERFACE $<BUILD_INTERFACE:${detail_header_files} ${header_files}>)
target_include_directories(circular_buffer INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>)
target_include_directories(circular_buffer SYSTEM INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)
target_include_directories(circular_buffer INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/config/include/>)

add_executable(tests_main ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)
target_link_libraries(tests_main circular_buffer)