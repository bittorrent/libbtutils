CFLAGS=-DPOSIX -DDEBUG -D_LIB
CXXFLAGS=$(CFLAGS) -std=c++11

SRC=src/*.cpp

all: ut_utils.so

ut_utils.so:
	g++ $(CXXFLAGS) -shared -o $@ $(SRC)
