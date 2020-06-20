cmake_minimum_required(VERSION 3.14)
project(poly_handle VERSION 1.0)

add_library(poly_handle INTERFACE)
target_include_directories(poly_handle SYSTEM INTERFACE include)
target_compile_features(poly_handle INTERFACE cxx_std_17)
option(POLY_TESTS "enable poly tests" OFF)
if (POLY_TESTS)
	include(FetchContent)
	FetchContent_Declare(doctest
		GIT_REPOSITORY https://github.com/onqtam/doctest.git
		GIT_TAG        2.3.7
		GIT_SHALLOW    TRUE
		GIT_PROGRESS   TRUE
	)
	FetchContent_MakeAvailable(doctest)


	add_executable(poly_test)
	target_sources(poly_test PRIVATE src/poly.cpp src/dt.cpp)
	target_link_libraries(poly_test PRIVATE poly_handle doctest::doctest)

	enable_testing()
	add_test(tests poly_test)

	add_executable(readme_code)
	target_sources(readme_code PRIVATE src/readme_code.cpp)
	target_link_libraries(readme_code PRIVATE poly_handle)
endif()
