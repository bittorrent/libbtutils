#pragma once

#ifdef WIN32

const char *inet_ntop(int af, const void *src, char *dest, size_t length);

#else

#include <arpa/inet.h> // for inet_ntop

#endif

