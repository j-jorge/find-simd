COMPILE_OPTIONS=-O3 -DNDEBUG \
	-I$(HOME)/.local/include \
	-L$(HOME)/.local/lib \
	-lbenchmark \
	-pthread

all: test-g++ test-clang++ test-icpc

test-g++: main.cpp
	g++ $^ -o $@ $(COMPILE_OPTIONS)

test-clang++: main.cpp
	clang++ $^ -o $@ $(COMPILE_OPTIONS)

test-icpc: main.cpp
	icpc $^ -o $@ $(COMPILE_OPTIONS)
