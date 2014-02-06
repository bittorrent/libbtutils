
// This is a standard function, but windows doesn't provide it
// that's why we only need this on windows
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdlib.h> // for NULL
#include <string.h> // for memcpy

#include "inet_ntop.h"

const char *inet_ntop(int af, const void *src, char *dest, size_t length)
{
	if (af != AF_INET && af != AF_INET6)
	{
		return NULL;
	}

	sockaddr_storage address;
	int address_length;

	if (af == AF_INET)
	{
		address_length = sizeof(sockaddr_in);
		sockaddr_in* ipv4_address = (sockaddr_in*)(&address);
		ipv4_address->sin_family = AF_INET;
		ipv4_address->sin_port = 0;
		memcpy(&ipv4_address->sin_addr, src, sizeof(in_addr));
	}
	else // AF_INET6
	{
		address_length = sizeof(sockaddr_in6);
		sockaddr_in6* ipv6_address = (sockaddr_in6*)(&address);
		ipv6_address->sin6_family = AF_INET6;
		ipv6_address->sin6_port = 0;
		ipv6_address->sin6_flowinfo = 0;
		// hmmm
		ipv6_address->sin6_scope_id = 0;
		memcpy(&ipv6_address->sin6_addr, src, sizeof(in6_addr));
	}

	DWORD string_length = (int)(length);
	int result;
	result = WSAAddressToStringA((sockaddr*)(&address),
								 address_length, 0, dest,
								 &string_length);

	// one common reason for this to fail is that ipv6 is not installed

	return result == SOCKET_ERROR ? NULL : dest;
}

#if defined _WIN32 && !defined _WIN32_WINNT
#error _WIN32_WINNT must be defined, otherwise we don't know which windows \
	version we're building for
#endif

#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x501

int inet_pton(int af, const char* src, void* dest)
{
	if (af != AF_INET && af != AF_INET6)
	{
		return -1;
	}

	SOCKADDR_STORAGE address;
	int address_length = sizeof(SOCKADDR_STORAGE);
	int result = WSAStringToAddressA((char*)(src), af, 0,
									 (sockaddr*)(&address),
									 &address_length);

	if (af == AF_INET)
	{
		if (result != SOCKET_ERROR)
		{
			sockaddr_in* ipv4_address =(sockaddr_in*)(&address);
			memcpy(dest, &ipv4_address->sin_addr, sizeof(in_addr));
		}
		else if (strcmp(src, "255.255.255.255") == 0)
		{
			((in_addr*)(dest))->s_addr = INADDR_NONE;
		}
	}
	else // AF_INET6
	{
		if (result != SOCKET_ERROR)
		{
			sockaddr_in6* ipv6_address = (sockaddr_in6*)(&address);
			memcpy(dest, &ipv6_address->sin6_addr, sizeof(in6_addr));
		}
	}

	return result == SOCKET_ERROR ? -1 : 1;
}
#endif

#endif // _WIN32

