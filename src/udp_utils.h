#ifndef __UDP_UTILS_H__
#define __UDP_UTILS_H__

#include "sockaddr.h"
#include "utypes.h"

#define ETHERNET_MTU 1500
#define IPV4_HEADER_SIZE 20
#define IPV6_HEADER_SIZE 40
#define UDP_HEADER_SIZE 8
#define GRE_HEADER_SIZE 24
#define PPPOE_HEADER_SIZE 8
#define MPPE_HEADER_SIZE 2
#define TEREDO_MTU 1280

#define UDP_IPV4_OVERHEAD (IPV4_HEADER_SIZE + UDP_HEADER_SIZE)
#define UDP_IPV6_OVERHEAD (IPV6_HEADER_SIZE + UDP_HEADER_SIZE)
#define UDP_TEREDO_OVERHEAD (UDP_IPV4_OVERHEAD + UDP_IPV6_OVERHEAD)

#define UDP_IPV4_MTU (ETHERNET_MTU - IPV4_HEADER_SIZE - UDP_HEADER_SIZE - GRE_HEADER_SIZE - PPPOE_HEADER_SIZE - MPPE_HEADER_SIZE)
#define UDP_IPV6_MTU (ETHERNET_MTU - IPV6_HEADER_SIZE - UDP_HEADER_SIZE - GRE_HEADER_SIZE - PPPOE_HEADER_SIZE - MPPE_HEADER_SIZE)
#define UDP_TEREDO_MTU (TEREDO_MTU - IPV6_HEADER_SIZE - UDP_HEADER_SIZE)

uint16 GetUDP_MTU(const SockAddr& remote);
uint16 GetUDP_Overhead(const SockAddr& remote);

class UDPSocketInterface {
	public:
		virtual void Send(const SockAddr& dest, cstr host, const byte *p, size_t len, uint32 flags = 0) = 0;
		virtual void Send(const SockAddr& dest, const byte *p, size_t len, uint32 flags = 0) = 0;
		virtual const SockAddr &GetBindAddr( void ) const = 0;
		virtual ~UDPSocketInterface();
};

#endif

