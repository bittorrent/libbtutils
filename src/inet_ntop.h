#pragma once

#ifdef WIN32

const char *inet_ntop(int af, const void *src, char *dest, size_t length);
int inet_pton(int af, const char* src, void* dest);

#else

#include <arpa/inet.h> // for inet_ntop

#endif

