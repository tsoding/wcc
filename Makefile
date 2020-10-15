WCC_CXXFLAGS=-Wall -Werror -Wextra -pedantic -std=c++17 -ggdb
WCC_LIBS=
NODE_FLAGS=--unhandled-rejections=strict --experimental-wasm-bigint

wcc: $(wildcard src/*.cpp) $(wildcard src/*.hpp)
	$(CXX) $(WCC_CXXFLAGS) -o wcc src/wcc.cpp $(WCC_LIBS)

.PHONY: test
test: test_add test_fib test_rot13_char

.PHONY: test_add
test_add: ./samples/test_add.js ./samples/add.wasm
	node $(NODE_FLAGS) ./samples/test_add.js

.PHONY: test_fib
test_fib: ./samples/test_fib.js ./samples/fib.wasm
	node $(NODE_FLAGS) ./samples/test_fib.js

.PHONY: test_rot13_char
test_rot13_char: ./samples/test_rot13_char.js ./samples/rot13_char.wasm
	node $(NODE_FLAGS) ./samples/test_rot13_char.js

# TODO: there is not test coverage
# TODO: ./samples/rot13_str.wc does not compile
# TODO: ./samples/primes.wc does not compile
# TODO: utilize gnu make wildcards in the build
.PHONY: samples
samples: ./samples/add.wasm ./samples/fib.wasm ./samples/rot13_char.wasm # ./samples/prime.wasm # ./samples/rot13_str.wasm

./samples/%.wasm: ./samples/%.wat
	wat2wasm -o $@ $<

./samples/%.wat: ./samples/%.wc wcc
	./wcc -t wat $< > $@

.PHONY: help
help:
	@echo '  make                          build ./wcc executable of the compiler in the current dir'
	@echo 'Samples:'
	@echo '  make samples                  build all of the samples in the ./samples/ dir'
	@echo '  make samples/add.wasm         build ./samples/add.wc sample'
	@echo '  make samples/fib.wasm         build ./samples/fib.wc sample'
	@echo '  make samples/rot13_char.wasm  build ./samples/rot13_char.wc sample'
	@echo '  make samples/rot13_str.wasm   build ./samples/rot13_str.wc sample'
	@echo 'Test:'
	@echo '  make test                     run all of the tests'
	@echo '  make test_add                 test ./samples/add.wc sample'
	@echo '  make test_fib                 test ./samples/fib.wc sample'
	@echo '  make test_rot13_char          test ./samples/rot13_char.wc sample'

# TODO: GoL sample
# TODO: Sample for https://en.wikipedia.org/wiki/Trial_division
