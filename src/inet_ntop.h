#pragma once

#ifdef WIN32

const char *inet_ntop(int af, const void *src, char *dest, size_t length);

#if defined(_WIN32_WINNT) && _WIN32_WINNT <= 0x501
int inet_pton(int af, const char* src, void* dest);
#endif

#else

#include <arpa/inet.h> // for inet_ntop

#endif

