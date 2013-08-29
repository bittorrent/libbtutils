#pragma once

#ifdef WIN32

const char *inet_ntop(int af, const void *src, char *dest, size_t length);

#if ((!defined NTDDI_VERSION) || (NTDDI_VERSION < NTDDI_LONGHORN))
int inet_pton(int af, const char* src, void* dest);
#endif

#else

#include <arpa/inet.h> // for inet_ntop

#endif

