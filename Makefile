WCC_CXXFLAGS=-Wall -Werror -pedantic -std=c++17 -ggdb
WCC_LIBS=

wcc: wcc.cpp
	$(CXX) $(WCC_CXXFLAGS) -o wcc wcc.cpp $(WCC_LIBS)
