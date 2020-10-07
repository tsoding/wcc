WCC_CXXFLAGS=-Wall -Werror -pedantic -std=c++17 -ggdb
WCC_LIBS=

wcc: $(wildcard src/*.cpp) $(wildcard src/*.hpp)
	$(CXX) $(WCC_CXXFLAGS) -o wcc src/wcc.cpp $(WCC_LIBS)
