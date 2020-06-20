MAKEFLAGS+=--no-print-directory
all: build/Makefile
	@cmake --build build -t all
test: build/Makefile
	@cmake --build build -t poly_test
	@cd build && ctest --output-on-failure
build/Makefile:
	@mkdir -p build && cmake -S. -Bbuild
.PHONY: all test
