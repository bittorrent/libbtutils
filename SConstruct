import os

cflags = '-DPOSIX -D_UNICODE -D_DEBUG -D_LIB -g -O0'

env = Environment(
		CCFLAGS = cflags,
		CXXFLAGS = cflags + ' -Wall -Werror -fno-strict-aliasing'
		)
#without this stock path would get used, thus /usr/bin/g++ would get invoked
env['ENV']['PATH'] = os.environ['PATH']

src = ['src/' + i for i in ('RefBase.cpp', 'bitfield.cpp', 'bloom_filter.cpp',
		'get_microseconds.cpp', 'inet_ntop.cpp', 'sockaddr.cpp', 'interlock.cpp',
		'snprintf.cpp', 'DecodeEncodedString.cpp', 'bencoding.cpp', 'bencparser.cpp')]
src_google_test = ['vendor/gtest-1.6.0/src/gtest-all.cc',
								'vendor/gmock-1.6.0/src/gmock-all.cc',
								'vendor/gmock-1.6.0/src/gmock_main.cc']
src_tests = src_google_test + ['unittests/' + i for i in ('TestBencEntity.cpp', 'TestBencoding.cpp')]

include_tests = ['src/', './', 'vendor/gtest-1.6.0/include',
									'vendor/gmock-1.6.0/include', 'vendor/gtest-1.6.0',
									'vendor/gmock-1.6.0']

Default(env.Library('ututils', src, CPPPATH=['src/']))
env.Program('unit_tests', src_tests, LIBS='ututils', LIBPATH='./',
		CPPPATH=include_tests)
Alias('test', 'unit_tests')
