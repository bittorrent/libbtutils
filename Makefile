CXX=g++
CFLAGS=-DPOSIX -D_DEBUG -D_LIB -g -O0
CXXFLAGS=$(CFLAGS) -Wall -Werror

SRC=$(addprefix src/, RefBase.cpp bitfield.cpp bloom_filter.cpp \
		get_microseconds.cpp inet_ntop.cpp sockaddr.cpp interlock.cpp snprintf.cpp)

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
LDFLAGS_UNITTESTS=-L./ -lututils_broken

all: libututils.so

bencoding: libututils_broken.so

test: unit_tests

libututils.so:
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRC)

libututils_broken.so:
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRC_BENCODING)

unit_tests: libututils_broken.so
	$(CXX) $(CXXFLAGS) $(INCLUDE_UNITTESTS) $(LDFLAGS_UNITTESTS) -o $@ $(SRC_UNITTESTS) 

clean:
	rm -f libututils.so libututils_broken.so unit_tests
