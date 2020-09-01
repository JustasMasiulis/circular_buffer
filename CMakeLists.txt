cmake_minimum_required(VERSION 3.2)
project(circular_buffer CXX)

# force set release
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

include ( cmake/Dependency.cmake )
include ( cmake/warnings.cmake )

add_library(circular_buffer INTERFACE)

target_include_directories( circular_buffer INTERFACE include )
target_compile_features( circular_buffer INTERFACE cxx_std_17 )
set_project_warinigs(circular_buffer)


add_executable(tests_main ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)
target_link_libraries(tests_main circular_buffer Catch2)

# catch integration for tests
include ("${CMAKE_CURRENT_BINARY_DIR}/_deps/catch2-src/contrib/ParseAndAddCatchTests.cmake")
include ("${CMAKE_CURRENT_BINARY_DIR}/_deps/catch2-src/contrib/Catch.cmake")
include(CTest)


#set (TEST_SOURCE_FILES
#		${PROJECT_SOURCE_DIR}/test/main.cpp)

#set target executable
add_executable (
	circular_buffer_test 
		test/main.cpp
	)

#add the library
target_link_libraries (circular_buffer_test circular_buffer Catch2)

enable_testing()
catch_discover_tests(circular_buffer_test)
#ParseAndAddCatchTests (circular_buffer_test)
