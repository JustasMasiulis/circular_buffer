cmake_minimum_required(VERSION 3.2)
project(circular_buffer CXX)

option (CIRCULAR_BUFFER_TESTING "Enable test library" ON)
option (CIRCULAR_BUFFER_EXAMPLE "Enable example library" ON)

# force set release
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

include ( cmake/Dependency.cmake )
include ( cmake/warnings.cmake )

add_library(circular_buffer INTERFACE)

target_include_directories( circular_buffer INTERFACE include )
target_compile_features( circular_buffer INTERFACE cxx_std_17 )
set_project_warinigs(circular_buffer)


if (CIRCULAR_BUFFER_TESTING)
	enable_testing()
	add_catch_and_benchmark()
	
	# catch integration for tests
	include ("${CMAKE_CURRENT_BINARY_DIR}/_deps/catch2-src/contrib/ParseAndAddCatchTests.cmake")
	include ("${CMAKE_CURRENT_BINARY_DIR}/_deps/catch2-src/contrib/Catch.cmake")
	include(CTest)

	#set target executable
	add_executable ( circular_buffer_test test/main.cpp )

	#add the library
	target_link_libraries (circular_buffer_test circular_buffer Catch2)
	catch_discover_tests(circular_buffer_test)
endif()
