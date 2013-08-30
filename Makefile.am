ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wall -DPOSIX -D_UNICODE -D_LIB

if DEBUG
AM_CFLAGS += -D_DEBUG -g -O0
else
AM_CFLAGS += -DNDEBUG -O2
endif

AM_CXXFLAGS = $(AM_CFLAGS)

lib_LIBRARIES = libut_utils.a
libut_utils_a_SOURCES = src/DecodeEncodedString.cpp		\
                        src/RefBase.cpp								\
                        src/bencoding.cpp							\
                        src/bencparser.cpp 						\
                        src/bitfield.cpp 							\
                        src/bloom_filter.cpp 					\
                        src/get_microseconds.cpp 			\
                        src/inet_ntop.cpp 						\
                        src/interlock.cpp 						\
                        src/snprintf.cpp 							\
                        src/sockaddr.cpp 							\
                        src/udp_utils.cpp
