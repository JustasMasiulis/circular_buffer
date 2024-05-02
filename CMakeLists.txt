cmake_minimum_required(VERSION 3.2)
project(CIRCULAR_BUFFER CXX)

set(header_files
	${PROJECT_SOURCE_DIR}/include/circular_buffer.hpp)

add_library(circular_buffer INTERFACE)

target_sources(circular_buffer INTERFACE $<BUILD_INTERFACE:${detail_header_files} ${header_files}>)
target_include_directories(circular_buffer INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>)
target_include_directories(circular_buffer SYSTEM INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)

option(JM_CIRCULAR_BUFFER_BUILD_TESTS "Build tests for circular buffer" ON)

if (JM_CIRCULAR_BUFFER_BUILD_TESTS)
	add_executable(tests_main ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)
	target_link_libraries(tests_main circular_buffer)

	# catch integration for tests

	set (CATCH_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/Catch/include")

	include ("${PROJECT_SOURCE_DIR}/Catch/contrib/ParseAndAddCatchTests.cmake")

	set (TEST_APP_NAME "circular_buffer_test")

	include_directories (${PROJECT_SOURCE_DIR}/include ${CATCH_INCLUDE_PATH})

	set (TEST_SOURCE_FILES
			${PROJECT_SOURCE_DIR}/test/main.cpp)

	#set target executable
	add_executable (${TEST_APP_NAME} ${TEST_SOURCE_FILES})

	#add the library
	target_link_libraries (${TEST_APP_NAME} circular_buffer)

	enable_testing()

	ParseAndAddCatchTests (${TEST_APP_NAME})
endif()
