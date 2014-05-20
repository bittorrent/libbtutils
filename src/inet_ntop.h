#ifndef INET_NTOP_H
#define INET_NTOP_H

const char *bt_inet_ntop(int af, const void *src, char *dest, size_t length);
int bt_inet_pton(int af, const char* src, void* dest);

#endif