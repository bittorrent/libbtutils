//#include "StdAfxCore.h"
#include "sockaddr.h"

#include <string.h> // for memcpy
#include <stdio.h> // for snprintf
#include <algorithm> // for std::min
#include "inet_ntop.h"
#include "endian_utils.h" // for ReadBE*() and WriteBE*()

// Set by Network_Initialize if system supports IPv6
bool SockAddr::_use_ipv6 = false;
in6_addr SockAddr::_in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
in6_addr SockAddr::_in6addr_any = IN6ADDR_ANY_INIT;

#if defined WIN32

#include <stdarg.h>

inline int snprintf(char* buf, int len, char const* fmt, ...)
{
   va_list lp;
   va_start(lp, fmt);
   int ret = _vsnprintf(buf, len, fmt, lp);
   va_end(lp);
   if (ret < 0) { buf[len-1] = 0; ret = len-1; }
   return ret;
}
#endif

/*
void sockaddr_from_bytes(sockaddr_storage* ss, uint8_t const* buf, int len)
{
	switch (len)
	{
		case 4:
		{
			ss->ss_family = AF_INET;
			sockaddr_in* ss4 = (sockaddr_in*)ss;
			memcpy(&ss4->sin_addr, buf, 4);
			break;
		}
		case 6:
		{
			ss->ss_family = AF_INET;
			sockaddr_in* ss4 = (sockaddr_in*)ss;
			memcpy(&ss4->sin_addr, buf, 4);
			memcpy(&ss4->sin_port, buf + 4, 2);
			break;
		}
		case 16:
		{
			ss->ss_family = AF_INET6;
			sockaddr_in6* ss6 = (sockaddr_in6*)ss;
			memcpy(&ss6->sin6_addr, buf, 16);
			break;
		}
		case 18:
		{
			ss->ss_family = AF_INET6;
			sockaddr_in6* ss6 = (sockaddr_in6*)ss;
			memcpy(&ss6->sin6_addr, buf, 16);
			memcpy(&ss6->sin6_port, buf + 16, 2);
			break;
		}
		default:
			memset(ss, 0, sizeof(sockaddr_storage));
	}
}

int sockaddr_to_bytes(sockaddr_storage const* ss, uint8_t* buf, bool with_port)
{
	if (ss->ss_family == AF_INET) {
		sockaddr_in const* ss4 = (sockaddr_in const*)ss;
		memcpy(buf, &ss4->sin_addr, 4);
		if (with_port) memcpy(buf + 4, &ss4->sin_port, 2);
		else return 4;
		return 6;
	}

	if (ss->ss_family == AF_INET6) {
		sockaddr_in6 const* ss6 = (sockaddr_in6 const*)ss;
		memcpy(buf, &ss6->sin6_addr, 16);
		if (with_port) memcpy(buf + 16, &ss6->sin6_port, 2);
		else return 16;
		return 18;
	}

	assert(false && "not supported");
	return 0;
}

int sockaddr_compare(sockaddr_storage const* lhs, sockaddr_storage const* rhs)
{
	if (lhs->ss_family != rhs->ss_family)
		return lhs->ss_family - rhs->ss_family;

	if (lhs->ss_family == AF_INET) {
		sockaddr_in const* lhs4 = (sockaddr_in const*)lhs;
		sockaddr_in const* rhs4 = (sockaddr_in const*)rhs;
		if (lhs4->sin_addr.s_addr != rhs4->sin_addr.s_addr)
			return lhs4->sin_addr.s_addr - rhs4->sin_addr.s_addr;
		return lhs4->sin_port - rhs4->sin_port;
	} else if (lhs->ss_family == AF_INET6) {
		sockaddr_in6 const* lhs6 = (sockaddr_in6 const*)lhs;
		sockaddr_in6 const* rhs6 = (sockaddr_in6 const*)rhs;
		int r = memcmp(&lhs6->sin6_addr, &rhs6->sin6_addr, sizeof(lhs6->sin6_addr));
		if (r != 0) return 0;
		return lhs6->sin6_port - rhs6->sin6_port;
	}

	assert(false && "not supported");
	return 0;
}

bool sockaddr_is_any(sockaddr_storage const* ss)
{
	if (ss->ss_family == AF_INET) {
		sockaddr_in const* ss4 = (sockaddr_in const*)ss;
		return ss4->sin_addr.s_addr == INADDR_ANY;
	}

	if (ss->ss_family == AF_INET6) {
		sockaddr_in6 const* ss6 = (sockaddr_in6 const*)ss;
		for (int i = 0; i < 16; ++i)
			if (ss6->sin6_addr.s6_addr[i] != 0) return false;
		return true;
	}

	assert(false && "not supported");
	return false;
}

int sockaddr_get_port(sockaddr_storage const* ss)
{
	if (ss->ss_family == AF_INET) {
		sockaddr_in const* ss4 = (sockaddr_in const*)ss;
		return ntohs(ss4->sin_port);
	}

	if (ss->ss_family == AF_INET6) {
		sockaddr_in6 const* ss6 = (sockaddr_in6 const*)ss;
		return ntohs(ss6->sin6_port);
	}

	assert(false && "not supported");
	return 0;
}

void sockaddr_set_port(sockaddr_storage* ss, int port)
{
	if (ss->ss_family == AF_INET) {
		sockaddr_in* ss4 = (sockaddr_in*)ss;
		ss4->sin_port = htons(port);
	}

	if (ss->ss_family == AF_INET6) {
		sockaddr_in6* ss6 = (sockaddr_in6*)ss;
		ss6->sin6_port = ntohs(port);
	}

	assert(false && "not supported");
}
*/

bool is_in_subnet(uint32 ip, uint32 subip, uint32 subnetmask)
{
	return (ip & subnetmask) == (subip & subnetmask);
}

bool is_ip_local(const SockAddr& sa)
{
	if (sa.isv6()) {
		const in6_addr a = sa.get_addr6();
		if (IN6_IS_ADDR_LOOPBACK(&a))
			return true;
		if (IN6_IS_ADDR_LINKLOCAL(&a))
			return true;
		if (IN6_IS_ADDR_SITELOCAL(&a))
			return true;
	}


	if (!sa.can_make_v4())
		return false;

	uint32 ip = sa.make_v4().get_addr4();

	// Local ranges:

	// 10.0.0.0/8
	static uint32 block10 = parse_ip("10.0.0.0");
	if (is_in_subnet(ip, block10,  0xFF000000))
		return true;

	// 192.168.0.0/16
	static uint32 block192 = parse_ip("192.168.0.0");
	if (is_in_subnet(ip, block192, 0xFFFF0000))
		return true;

	// 169.254.0.0/16
	static uint32 block169 = parse_ip("169.254.0.0");
	if (is_in_subnet(ip, block169, 0xFFFF0000))
		return true;

	// 172.16.0.0/12
	static uint32 block172 = parse_ip("172.16.0.0");
	if (is_in_subnet(ip, block172, 0xFFF00000))
		return true;

	// 127.0.0.1/8
	static uint32 block127 = parse_ip("127.0.0.0");
	if (is_in_subnet(ip, block127, 0xFF000000))
		return true;

	return false;
}

// Why not use inet_addr or WSAStringToAddress? Because they do not handle "012.034.056.078"
uint32 parse_ip(cstr ip, bool *valid)
{
	uint32 r = 0;
	uint n;
	str end;

	if (valid) *valid=false;

	if (ip == NULL) {
		assert(ip);
		return (uint32)-1;
	}

	for(uint i = 0; i != 4; i++) {
		n = strtoul(ip, &end, 10);
		if (n > 255) return (uint32)-1;
		ip = end;
		if (*ip++ != (i==3 ? 0 : '.')) return (uint32)-1;
		r = (r<<8) + n;
	}

	if (valid) *valid=true;
	return r;
}

in6_addr parse_ip_v6(cstr ip_v6, bool *valid)
{
	in6_addr a = IN6ADDR_ANY_INIT;
	int r = inet_pton(AF_INET6, ip_v6, &a);
	if (valid)
		*valid = (bool)(r == 1);
	return a;
}

#ifndef WIN32
SockAddr SockAddr::make_mapped_v4() const
{
	if (is_mapped_v4())
		return *this;

	assert(isv4());

	if (!isv4())
		return SockAddr(INADDR_ANY, 0).make_mapped_v4();

	SockAddr tmp;
	tmp.set_family(AF_INET6);
	tmp._sin6d[0] = 0;
	tmp._sin6d[1] = 0;
	tmp._sin6w[4] = 0;
	tmp._sin6w[5] = 0xffff;
	tmp._port = _port;
	tmp._sin4 = _sin4;
	return tmp;
}
#endif

#if !defined WIN32 || defined __WINE__
// True if this is a mapped v4 address
bool SockAddr::is_mapped_v4() const
{
	if (get_family() != AF_INET6)
		return false;
#if defined __WINE__
	// IN6_IS_ADDR_V4MAPPED macro not defined on Wine
	return _sin6d[0] == 0 && _sin6d[1] == 0 && _sin6w[4] == 0 && _sin6w[5] == 0xffff;
#else
	return IN6_IS_ADDR_V4MAPPED((in6_addr*)_sin6) != 0;
#endif // __WINE__
}
#endif

#if !defined WIN32 || !defined _MSC_VER
SockAddr SockAddr::make_v4() const
{
	if (isv4())
		return *this;

	assert(is_mapped_v4());

	if (!is_mapped_v4())
		return SockAddr(0,0);

	return SockAddr(ntohl(_sin4), _port);
}
#endif

bool SockAddr::is_loopback() const
{
	// XXX Maybe also check for local i/f addresses here
	if (isv4())
		return get_addr4() == INADDR_LOOPBACK;

	if (is_mapped_v4())
		return make_v4().is_loopback();

	return !memcmp(&_sin6, &SockAddr::_in6addr_loopback, sizeof(_sin6));
}


bool SockAddr::operator==(const SockAddr& rhs) const
{
	if (&rhs == this)
		return true;

	if (get_port() != rhs.get_port())
		return false;

	if (get_family() != rhs.get_family())
		return false;

	return ip_eq(rhs);
}

int SockAddr::ip_compare(const SockAddr& rhs) const
{
	// Both need to be either v4/mapped or v6 to be equal
	const int i = can_make_v4() - rhs.can_make_v4();
	if (i) return i;

	// If both are mapped or v4, compare v4 addr
	if (can_make_v4()) {
		return make_v4().get_addr4() - rhs.make_v4().get_addr4();
	}

	// Otherwise compare v6 addr
	assert(get_family() == AF_INET6);
	assert(get_family() == rhs.get_family());

	return memcmp(_sin6, rhs._sin6, sizeof(_sin6));
}

int SockAddr::compare(const SockAddr& rhs) const
{
	if (&rhs == this)
		return 0;

	const int i = ip_compare(rhs);
	if (i) return i;

	return _port - rhs._port;
}

SockAddr::SockAddr()
{
	if (_use_ipv6) {
		set_family(AF_INET6);
		memcpy(_sin6, &SockAddr::_in6addr_any, sizeof(_sin6));
	} else {
		set_family(AF_INET);

		// Fill in mapped-v4 prefix on non-Win32 so we get a
		// fixed key len for hashing
		_sin6d[0] = 0;
		_sin6d[1] = 0;
		_sin6w[4] = 0;
		_sin6w[5] = 0xffff;
		_sin4 = INADDR_ANY;
	}
	_port = INVALID_PORT;
}


SockAddr::SockAddr(uint32 addr, uint16 port)
{
	set_family(AF_INET);

	// Fill in mapped-v4 prefix on non-Win32 so we get a fixed key
	// len for hashing
	_sin6d[0] = 0;
	_sin6d[1] = 0;
	_sin6w[4] = 0;
	_sin6w[5] = 0xffff;
	_sin4 = htonl(addr);
	_port = port;
}


SockAddr::SockAddr(const in6_addr& addr, uint16 port)
{
	set_family(AF_INET6);
	memcpy(_sin6, &addr, sizeof(_sin6));
	_port = port;

	if (is_mapped_v4())
		set_family(AF_INET);
}


SockAddr::SockAddr(const SOCKADDR_STORAGE& sa)
{
	set_family(sa.ss_family);
	if (sa.ss_family == AF_INET) {
		const sockaddr_in& sin = (sockaddr_in&)sa;
		_sin6d[0] = 0;
		_sin6d[1] = 0;
		_sin6w[4] = 0;
		_sin6w[5] = 0xffff;
		_sin4 = sin.sin_addr.s_addr;
		_port = ntohs(sin.sin_port);
	} else {
		const sockaddr_in6& sin6 = (sockaddr_in6&)sa;
		_port = ntohs(sin6.sin6_port);
		_in._in6addr = sin6.sin6_addr;
	}
}

SockAddr SockAddr::parse_addr(cstr addrspec, bool* valid)
{
	if (addrspec == NULL) {
		if (valid) *valid = false;
		return SockAddr();
	}

	bool ok = false;
	SockAddr retval;
	uint16 portval = 0;

	if (*addrspec == '[') {
		// IPv6, rfc2732 style
		char addr[200];
		strncpy(addr, addrspec, sizeof(addr));
		str end = (str)strrchr(addr, ']');
		if (end) {
			*end++ = 0;
			const in6_addr ip6 = parse_ip_v6(addr+1, &ok);
			if (ok) {
				if (*end == ':') {
					// with port
					end++;
					portval = atoi(end); // xxx use strtol
				}
				retval = SockAddr(ip6, portval);
			}
		}
	} else {
		uint32 ip4;
		cstr port = strrchr(addrspec, ':');

		if (!port) {
			// IPv4
			ip4 = parse_ip(addrspec, &ok);
			if (ok) {
				retval = SockAddr(ip4, 0);
			}
		} else {
			cstr fwd = strchr(addrspec, ':');
			if (fwd == port) {
				// IPv4 with port
				size_t len = port - addrspec;
				char addr[400];
				memcpy(addr, addrspec, (std::min)(len + 1, sizeof(addr)));
				addr[len] = 0;
				ip4 = parse_ip(addr, &ok);
				if (ok) {
					port++;
					portval = atoi(port); // xxx use strtol
					retval = SockAddr(ip4, portval);
				}
			} else {
				// IPv6, rfc3513 style
				const in6_addr ip6 = parse_ip_v6(addrspec, &ok);
				if (ok) {
					retval = SockAddr(ip6, 0);
				}
			}
		}
	}

	if (valid) *valid = ok;
	return retval;
}

size_t SockAddr::compact(byte* p, bool with_port) const
{
	if (isv4()) {
		WriteBE32(p, get_addr4());
		if (with_port) {
			WriteBE16(p+4, get_port());
			return 6;
		}
		return 4;
	} else {
		in6_addr addr6 = get_addr6();
		memcpy(p, &addr6, sizeof(in6_addr));
		if (with_port) {
			WriteBE16(p+16, get_port());
			return 18;
		}
		return 16;
	}
}

bool SockAddr::from_compact(const byte* p, size_t len)
{
	_port = 0;
	switch (len) {

	case 6: {
		_port = ReadBE16(p+4);
	}
	// fall-through
	case 4: {
		set_family(AF_INET);
		// Fill in mapped-v4 prefix on non-Win32 so we get a fixed key
		// len for hashing
		_sin6d[0] = 0;
		_sin6d[1] = 0;
		_sin6w[4] = 0;
		_sin6w[5] = 0xffff;
		_sin4 = htonl(ReadBE32(p));
		return true;
	}

	case 18: {
		_port = ReadBE16(p+16);
	}
	// fall-through
	case 16: {
		set_family(AF_INET6);
		memcpy(_sin6, (const struct in6_addr*)p, sizeof(_sin6));
		return true;
	}

	default:
		memset(_sin6, 0, sizeof(_sin6));
	}

	return false;
}

SockAddr::SockAddr(const byte* p, size_t len, bool* success)
{
	bool s = from_compact(p, len);
	if (success)
		*success = s;
}

bool ParseCIDR(cstr s, SockAddr *pfrom, SockAddr *pto)
{
	str p = (str)strchr(s, '/');
	if (!p) return false;
	*p++ = '\0';

	bool valid  = false;
	SockAddr from = SockAddr::parse_addr(s, &valid);
	if (!valid) return false;

	SockAddr to;
	if (from.isv4()) {
		uint32 ip = from.get_addr4();
		int size = strtol(p, NULL, 10);
		for (int i = 0; i < (32 - size); i++) {
			ip |= 1<<i;
		}
		to.set_addr4(ntohl(ip));
	} else {
		assert(0);
	}

	if (pfrom) *pfrom = from;
	if (pto) *pto = to;
	return true;
}

char* SockAddr::get_arpa() const
{
	char buf[500];

	if (isv4()) {
		// 1.2.3.4
		// is
		// 4.3.2.1.in-addr.arpa
		uint32 a = get_addr4();
		char const* out = (char*)&a;
		snprintf(buf, sizeof(buf), "%d.%d.%d.%d.in-addr.arpa",
			out[3], out[2], out[1], out[0]);
	}
	else
	{
		// 2001:db8::567:89ab
		// is:
		// b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa.
		in6_addr a = get_addr6();
#define out a.s6_addr
		snprintf(buf, sizeof(buf), "%x.%x.%x.%x."
			"%x.%x.%x.%x."
			"%x.%x.%x.%x."
			"%x.%x.%x.%x."
			"%x.%x.%x.%x."
			"%x.%x.%x.%x."
			"%x.%x.%x.%x."
			"%x.%x.%x.%x."
			"ip6.arpa",
			out[31], out[30], out[29], out[28],
			out[27], out[26], out[25], out[24],
			out[23], out[22], out[21], out[20],
			out[19], out[18], out[17], out[16],
			out[15], out[14], out[13], out[12],
			out[11], out[10], out[9], out[8],
			out[7], out[6], out[5], out[4],
			out[3], out[2], out[1], out[0]);
#undef out
	}
	return strdup(buf);
}

SockAddr SockAddr::round_up(SockAddr ip, const SockAddr &mask)
{
	int m = 1;
	for (int i = sizeof(ip._in._in6) - 1; m && i >= (ip.isv6() ? 0 : sizeof(ip._in._in6) - sizeof(in_addr)); i--) {
		byte invmask = ~mask._in._in6[i];
		m = (int)(invmask | ip._in._in6[i]) + 1;
		ip._in._in6[i] = m;
		m >>= 8;
	}
	return ip;
}
