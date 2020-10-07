WCC_CXXFLAGS=-Wall -Werror -pedantic -std=c++17 -ggdb
WCC_LIBS=

.PHONY: all
all: wcc

wcc: $(wildcard src/*.cpp) $(wildcard src/*.hpp)
	$(CXX) $(WCC_CXXFLAGS) -o wcc src/wcc.cpp $(WCC_LIBS)

./samples/add.wasm: ./samples/add.wat
	wat2wasm -o ./samples/add.wasm ./samples/add.wat

./samples/add.wat: ./samples/add.wc wcc
	./wcc -t wat ./samples/add.wc > ./samples/add.wat

./samples/fib.wasm: ./samples/fib.wat
	wat2wasm -o ./samples/fib.wasm ./samples/fib.wat

./samples/fib.wat: ./samples/fib.wc wcc
	./wcc -t wat ./samples/fib.wc > ./samples/fib.wat

.PHONY: samples
# TODO: rot13_char.wc sample is not compilable
samples: ./samples/add.wasm ./samples/fib.wasm # ./samples/rot13_char.wasm

./samples/rot13_char.wasm: ./samples/rot13_char.wat
	wat2wasm -o ./samples/rot13_char.wasm ./samples/rot13_char.wat

./samples/rot13_char.wat: ./samples/rot13_char.wc wcc
	./wcc -t wat ./samples/rot13_char.wc > ./samples/rot13_char.wat

# TODO: who to run the samples?
