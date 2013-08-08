CXX=g++
CFLAGS=-DPOSIX -DDEBUG -D_LIB
CXXFLAGS=$(CFLAGS) -std=c++11 -Wall -Werror

SRC=src/RefBase.cpp src/bitfield.cpp src/bloom_filter.cpp src/get_microseconds.cpp \
		src/inet_ntop.cpp src/sockaddr.cpp src/interlock.cpp src/snprintf.cpp

SRC_BENCODING=$(SRC) src/bencoding.cpp src/bencparser.cpp

SRC_TESTS=unittests/TestBencEntity.cpp unittests/TestBencoding.cpp

INCLUDE_TESTS=-Isrc/ -I./ -Ivendor/gtest-1.6.0/include -Ivendor/gmock-1.6.0/include -Ivendor/gtest-1.6.0 -Ivendor/gmock-1.6.0
LDFLAGS_TESTS=-L./ -lut_utils_broken.so

all: ut_utils.so

bencoding: ut_utils_broken.so

test: unit_tests

ut_utils.so:
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRC)

ut_utils_broken.so:
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRC_BENCODING)

unit_tests:
	$(CXX) $(CXXFLAGS) $(INCLUDE_TESTS) -o $@ $(SRC_TESTS) 
