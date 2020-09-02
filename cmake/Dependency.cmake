include ( cmake/get_cpm.cmake )

function (add_catch_and_benchmark)
	CPMAddPackage(
	  NAME Catch2
	  GITHUB_REPOSITORY catchorg/Catch2
	  VERSION 2.5.0
	)
	
	CPMAddPackage(
	  NAME benchmark
	  GITHUB_REPOSITORY google/benchmark
	  VERSION 1.4.1
	  OPTIONS
		"BENCHMARK_ENABLE_TESTING OFF"
		"BENCHMARK_ENABLE_EXCEPTIONS OFF"
		"BENCHMARK_ENABLE_GTEST_TESTS OFF"
		"BENCHMARK_ENABLE_INSTALL OFF"
	)

	if (benchmark_ADDED)
	  # compile with C++17
	  set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
	endif()
endfunction()


