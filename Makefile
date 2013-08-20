CXX=g++
CFLAGS=-DPOSIX -D_UNICODE -D_DEBUG -D_LIB -g -O0
CXXFLAGS=$(CFLAGS) -Wall -Werror
#CXXFLAGS=$(CFLAGS) -std=c++11 -Wall -Werror

SRC=$(addprefix src/, RefBase.cpp bitfield.cpp bloom_filter.cpp \
	get_microseconds.cpp inet_ntop.cpp sockaddr.cpp interlock.cpp snprintf.cpp \
	DecodeEncodedString.cpp \
)

SRC_BENCODING=$(SRC) src/bencoding.cpp src/bencparser.cpp

SRC_TESTS=$(addprefix unittests/, TestBencEntity.cpp TestBencoding.cpp)

SRC_GOOGLE_TEST = \
	vendor/gtest-1.6.0/src/gtest-all.cc \
	vendor/gmock-1.6.0/src/gmock-all.cc \
	vendor/gmock-1.6.0/src/gmock_main.cc 

SRC_UNITTESTS = \
	$(SRC_GOOGLE_TEST) \
	$(sort $(addprefix unittests/, TestBencEntity.cpp TestBencoding.cpp) )

INCLUDE_UNITTESTS=-Isrc/ -I./ -Ivendor/gtest-1.6.0/include -Ivendor/gmock-1.6.0/include -Ivendor/gtest-1.6.0 -Ivendor/gmock-1.6.0
LDFLAGS_UNITTESTS=-L./ -lututils

all: libututils.so

test: unit_tests

libututils.so:
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRC_BENCODING)

unit_tests: libututils.so
	$(CXX) $(CXXFLAGS) -std=c++11 $(INCLUDE_UNITTESTS) $(LDFLAGS_UNITTESTS) -o $@ $(SRC_UNITTESTS) 

clean:
	rm -f libututils.so unit_tests
