WCC_CXXFLAGS=-Wall -Werror -pedantic -std=c++17 -ggdb
WCC_LIBS=

all: wcc ./samples/add.wasm ./samples/fib.wasm

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
