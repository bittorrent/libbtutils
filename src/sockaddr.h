#ifndef __SOCKADDR_H__
#define __SOCKADDR_H__

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
typedef sockaddr_storage SOCKADDR_STORAGE;
#endif // _WIN32

#include <stdlib.h> // for NULL
#include <assert.h>
#include <string.h> // for memcmp
#include "target.h"
#include "utypes.h"

#define INVALID_PORT 0

/*
int sockaddr_compare(sockaddr_storage const* lhs, sockaddr_storage const* rhs);
bool sockaddr_is_any(sockaddr_storage const* ss);
int sockaddr_get_port(sockaddr_storage const* ss);
void sockaddr_set_port(sockaddr_storage* ss, int port);
int sockaddr_to_bytes(sockaddr_storage const* ss, uint8_t* buf, bool with_port);
void sockaddr_from_bytes(sockaddr_storage* ss, uint8_t const* buf, int len);
*/

uint32 parse_ip(cstr ip, bool *valid = NULL);
in6_addr parse_ip_v6(cstr ip_v6, bool *valid = NULL);

struct PACKED SockAddr {
	// If this is set, we use IPv6 addresses internally, but send and
	// receive v4 over protocols that require it.  getv4addr() will
	// return an address suitable for this; it requires the SockAddr
	// to either be a v4 address or a mapped v4 addr.  If this is
	// false, any attempt to create an IPv6 address will fail.  (Assert
	// in debug, return ::0 in release.)
	static bool _use_ipv6;

	static in6_addr _in6addr_loopback;
	static in6_addr _in6addr_any;

	// The values are always stored here in network byte order, while all
	// functions expect and return host byte order.

	union PACKED {
		byte _in6[16];		// IPv6
		uint16 _in6w[8];	// IPv6, word based (for convenience)
		uint32 _in6d[4];	// Dword access
		in6_addr _in6addr;	// For convenience
	} _in;

	uint16 _port;

#define _sin4 _in._in6d[3]	// IPv4 is stored where it goes if mapped

#define _sin6 _in._in6
#define _sin6w _in._in6w
#define _sin6d _in._in6d

#if defined _WIN32 && defined _MSC_VER
	// Winsock2 doesn't support mapped v4, so we use mapping to
	// store v4 addresses and infer family from the prefix.
	int get_family() const { return is_mapped_v4() ? AF_INET : AF_INET6; }

	// all v4 addresses are stored mapped, otherwise it's v6, so setting
	// the family does nothing
	void set_family(byte) { }

	// True if this is or can be converted to v4
	bool can_make_v4() const { return is_mapped_v4(); }

	// Make it an IPv4 address
	SockAddr make_v4() const { assert(isv4()); return *this; }

	// Make it an IPv6 address
	SockAddr make_v6() const { assert(isv6()); return *this; }

	// Return a mapped IPv4 address from AF_INET address
	SockAddr make_mapped_v4() const { assert(0); }
#else
	byte _family;

	int get_family() const { return _family; }
	void set_family(byte arg) { _family = arg; }

	// True if this is or can be converted to v4
	bool can_make_v4() const { return isv4() || is_mapped_v4(); }

	// Make it an IPv4 address
	SockAddr make_v4() const;

	// Make it an IPv6 address
	SockAddr make_v6() const { return isv4() ? make_mapped_v4() : *this; }

	// Return a mapped IPv4 address from AF_INET address
	SockAddr make_mapped_v4() const;
#endif

	// True if this is a v4 (or v6) address
	bool isv4() const { return get_family() == AF_INET; }
	bool isv6() const { return get_family() == AF_INET6; }
	bool isteredo() const
	{
		// teredo addresses starts with 2001::0000::
		return isv6() && _sin6[0] == 20
			&& _sin6[1] == 1 && _sin6[2] == 0 && _sin6[3] == 0;
	}

	// True if this is a mapped v4 address
	bool is_mapped_v4() const
#if defined _WIN32 && defined _MSC_VER
	{
		return IN6_IS_ADDR_V4MAPPED((in6_addr*)_sin6) != 0;
	}
#else
	; // defined in sockaddr.cpp: avoid compiler warnings
#endif

	// Return v4 address portion in host byte order
	uint32 get_addr4() const
	{
#if !defined _WIN32
		if (is_mapped_v4())
			return make_v4().get_addr4();
#endif
		return ntohl(_sin4);
	}

	// Return v6 address portion in network byte order
	in6_addr get_addr6() const { return _in._in6addr; }

	void set_addr4(uint32 addr)
	{
		make_v4();
		_sin4 = addr;
	}

	void set_addr6(const in6_addr &in6addr)
	{
		_in._in6addr = in6addr;
	}

	// Return something to hash with
	const void* get_hash_key() const { return (void*)_sin6; }
	static int get_hash_key_len() { return 16; }

	// Return port number, host byte order.
	uint16 get_port() const { return _port; }

	// Set port
	void set_port(const uint16 port) { _port = port; }

	// True if local
	bool is_loopback() const;

	// True if INADDR_ANY/IN6ADDR_ANY
	bool is_addr_any() const
	{
		return isv4() ? _sin4 == INADDR_ANY : !memcmp(&_in6addr_any, _sin6, sizeof(_sin6));
	}

	SOCKADDR_STORAGE get_sockaddr_storage(socklen_t* len = NULL) const
	{
		SOCKADDR_STORAGE sa;
		if (isv4()) {
			sockaddr_in& sin = (sockaddr_in&)sa;
			if (len) *len = sizeof(sin);
			memset(&sin, 0, sizeof(sin));
#if defined(NDEBUG) && BT_LITTLE_ENDIAN && defined(BROKEN_OPTIMIZED_HTONS) // Could use __OPTIMIZE__ instead of NDEBUG
			// Don't have a big-endian device that exhibits the problem so fix is little-endian-only
			unsigned int tmp;
			// Using htons_inline here doesn't change the broken outcome
			tmp = (((_port >> 8) | (_port << 8)) << 16) + AF_INET;
			memcpy(&sa, &tmp, sizeof(tmp));
#else
			sin.sin_family = AF_INET;
			sin.sin_port = htons(_port);
#endif
			sin.sin_addr.s_addr = _sin4;
		} else {
			sockaddr_in6& sin6 = (sockaddr_in6&)sa;
			memset(&sin6, 0, sizeof(sin6));
			if (len) *len = sizeof(sin6);
			sin6.sin6_family = AF_INET6;
			sin6.sin6_addr = _in._in6addr;
			sin6.sin6_port = htons(_port);
		}
		return sa;
	}

	// Address parser, accept input of any of the following formats:
	// 127.0.0.1		 ipv4, port = 0
	// 127.0.0.1:6881	ipv4
	// ::1		   ipv6, port = 0
	// [::1]:6881		ipv6
	static SockAddr parse_addr(cstr addrspec, bool *valid = NULL);

	// returns size of compact format
	size_t compact(byte* p, bool with_port) const;
	// takes size of compact format
	bool from_compact(const byte* p, size_t len);

	char* get_arpa() const; // allocates

	// Weak equality (a mapped v4 address will equal a v4 addr)
	bool operator==(const SockAddr& rhs) const;
	bool operator!=(const SockAddr& rhs) const { return !(*this == rhs); }

	bool ip_eq(const SockAddr& rhs) const { return memcmp(&_in, &rhs._in, sizeof(_in)) == 0; }
	int64 ip_compare(const SockAddr& rhs) const;

	static SockAddr round_up(SockAddr ip, const SockAddr &mask);

	// Weak ordering
	bool operator<(const SockAddr& rhs) const { return compare(rhs) < 0; }
	bool operator>(const SockAddr& rhs) const { return compare(rhs) > 0; }
	bool operator>=(const SockAddr& rhs) const { return compare(rhs) >= 0; }
	bool operator<=(const SockAddr& rhs) const { return compare(rhs) <= 0; }

	int64 compare(const SockAddr& rhs) const;

	// Default constructor.  If v6 isn't enabled, creates a v4 (0,0) endpoint.
	// Otherwise creates a v6 (::0,0) endpoint.
	SockAddr();

	// Construct from v4 addr, port (host byte order)
	SockAddr(uint32 addr, uint16 port);

	// Construct from v6 addr, port (host byte order, except in6_addr)
	SockAddr(const in6_addr& addr, uint16 port);

	// Construct from SOCKADDR_STORAGE
	SockAddr(const SOCKADDR_STORAGE& sa);

	// Construct from "compact" allocation
	SockAddr(const byte* p, size_t len, bool* success);
};

bool is_ip_local(const SockAddr& ip);
bool is_in_subnet(uint32 ip, uint32 subip, uint32 subnetmask);

static inline const SockAddr* basic_fmt_arg(const SockAddr& arg) { return &arg; }
static inline cstr basic_fmt(const SockAddr&) { return "%A"; }
static inline ctstr basic_tfmt(const SockAddr& sa) { return _T("%A"); }

bool ParseCIDR(cstr s, SockAddr *pfrom, SockAddr *pto);

struct
#ifndef LEAKCHECK
	PACKED
#endif
	TinyAddr {
	union {
		uint32 ip;
		SockAddr* sa;
	} _addr;
	uint16 _port; // if 0, use _addr.sa, otherwise use _addr.ip

	TinyAddr() {
		_port = 1;
		_addr.ip = 0;
	}

	TinyAddr(const SockAddr& sa) {
		_port = 1;
		*this = sa;
	}

	operator SockAddr() const {
		if (_port) {
			return SockAddr(_addr.ip, _port);
		} else {
			return *_addr.sa;
		}
	}

	TinyAddr& operator= (const SockAddr& sa) {
		if (_port == 0) {
			delete _addr.sa;
		}

		if (sa.isv4() && sa.get_port() != 0) {
			_port = sa.get_port();
			_addr.ip = sa.get_addr4();
		} else {
			_port = 0;
			_addr.sa = new SockAddr(sa);
		}

		return *this;
	}

	uint16 get_port() const { return (_port == 0 ? _addr.sa->get_port() : _port); };

	void set_port(uint16 port) {
		if (_port == 0) {
			_addr.sa->set_port(port);
		} else if (port == 0) {
			SockAddr addr = (SockAddr)*this;
			addr.set_port(port);
			*this = addr;
		} else {
			_port = port;
		}
	}

	bool isv4() const { return (_port != 0 || _addr.sa->isv4()); }
	bool isv6() const { return (_port == 0 && _addr.sa->isv6()); }
	bool is_addr_any() const { return (_port == 0 ? _addr.sa->is_addr_any() : _addr.ip == INADDR_ANY); }
	uint32 get_addr4() const { return (_port == 0 ? _addr.sa->get_addr4() : _addr.ip); };
	in6_addr get_addr6() const { return (_port == 0 ? _addr.sa->get_addr6() : ((SockAddr)*this).get_addr6()); }
	bool ip_eq(const SockAddr& rhs) const { return (_port == 0 ? _addr.sa->ip_eq(rhs) : rhs.isv4() && _addr.ip == get_addr4()); }

	bool operator== (const SockAddr& rhs) const {
		if (get_port() != rhs.get_port()) {
			return false;
		}

		if (isv4()) {
			return rhs.isv4() && get_addr4() == rhs.get_addr4();
		} else if (isv6()) {
			in6_addr a = get_addr6();
			in6_addr b = rhs.get_addr6();

			return rhs.isv6() && !memcmp(&a, &b, sizeof(a));
		}

		return false;
	}

	~TinyAddr() {
		if (_port == 0) {
			delete _addr.sa;
		}
	}

private:
	// no copying for now
	TinyAddr(const TinyAddr& ta) {}
	TinyAddr& operator= (const TinyAddr& ta) { return *this; }
};

static inline const TinyAddr* basic_fmt_arg(const TinyAddr& arg) { return &arg; }
static inline cstr basic_fmt(const TinyAddr&) { return "%T"; }
static inline ctstr basic_tfmt(const TinyAddr& sa) { return _T("%T"); }

#endif //__SOCKADDR_H__

