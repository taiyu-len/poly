MAKEFLAGS+=--no-print-directory
all: build/Makefile compile_commands.json
	@cmake --build build -t all
test: build/Makefile
	@cmake --build build -t poly_test
	@cd build && ctest --output-on-failure
build/Makefile:
	@mkdir -p build && cmake -S. -Bbuild
compile_commands.json: build/Makefile
	@ln -sr build/compile_commands.json .
.PHONY: all test
