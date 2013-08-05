CXX=g++
CFLAGS=-DPOSIX -DDEBUG -D_LIB
CXXFLAGS=$(CFLAGS) -std=c++11

SRC=src/RefBase.cpp src/bitfield.cpp src/bloom_filter.cpp src/get_microseconds.cpp \
		src/inet_ntop.cpp src/sockaddr.cpp src/interlock.cpp src/snprintf.cpp

SRC_BENCODING=$(SRC) src/bencoding.cpp src/bencparser.cpp

all: ut_utils.so

bencoding: ut_utils_broken.so

ut_utils.so:
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRC)

ut_utils_broken.so:
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRC_BENCODING)
