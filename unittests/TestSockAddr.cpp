#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "sockaddr.h"

//
// This test was converted from the old utassert style unit test.
// It lacks testing of IPv6 because a detection function doesn't exist
// in this unit set.
// TODO:  Must initialize network on Windows for the parse_ip_v6 function to work
//

static const char * const ADDR_V4_LOW = "10.19.40.81";
static const char * const ADDR_V4_MED = "10.20.40.80";
static const char * const ADDR_V4_HIGH = "10.20.40.81";
static const uint16 TEST_PORT = 1234;
static const char * const LOOPBACK_V4 = "127.0.0.1";
static const char * const LOOPBACK_V6 = "::1";
static const uint32 TEST_V4_ADDR_ANY = 0;
static const char * const LOOPBACK_V4_WITH_TEST_PORT = "127.0.0.1:1234";
static const char * const LOOPBACK_V6_WITH_TEST_PORT = "[::1]:1234";
static const char * const ANY_V4_WITH_TEST_PORT = "0.0.0.0:1234";
static const char * const ADDR_V4_MED_WITH_TEST_PORT = "10.20.40.80:1234";

enum AddrType
{
	AT_NORMAL = 0,
	AT_LOOPBACK = 1,
	AT_ANY = 2
};

static void sockaddr_test_v4(const SockAddr & sa_v4, const uint32 addr, const uint16 port, const AddrType addr_type)
{
	EXPECT_TRUE(sa_v4.isv4());
	EXPECT_TRUE(!sa_v4.isv6());
	EXPECT_TRUE(port == sa_v4.get_port());
	EXPECT_TRUE(addr == sa_v4.get_addr4());
	EXPECT_TRUE(AT_NORMAL == addr_type || AT_LOOPBACK == addr_type || AT_ANY == addr_type);
	switch (addr_type)
	{
	   case AT_NORMAL:
		EXPECT_TRUE(!sa_v4.is_loopback());
		EXPECT_TRUE(!sa_v4.is_addr_any());
		break;

	   case AT_LOOPBACK:
		EXPECT_TRUE(sa_v4.is_loopback());
		EXPECT_TRUE(!sa_v4.is_addr_any());
		break;

	   case AT_ANY:
		EXPECT_TRUE(!sa_v4.is_loopback());
		EXPECT_TRUE(sa_v4.is_addr_any());
		break;
	}
}

static void sockaddr_construct_v4(const uint32 addr, const uint16 port, const AddrType addr_type)
{
	SockAddr sa_v4(addr, port);
	sockaddr_test_v4(sa_v4, addr, port, addr_type);
}

static void sockaddr_construct_v4(const char * const addrport, const uint32 addr, const uint16 port, const AddrType addr_type)
{
	bool is_valid = false;
	SockAddr sa_v4 = SockAddr::parse_addr(addrport, &is_valid);
	EXPECT_TRUE(is_valid);
	sockaddr_test_v4(sa_v4, addr, port, addr_type);
}

TEST(SockAddr, TestConstruction)
{
	// Default constructor
	SockAddr sa_default;
	EXPECT_TRUE(!sa_default.isv6());
	EXPECT_TRUE(sa_default.isv4());
	EXPECT_TRUE(AF_INET6 != sa_default.get_family());
	EXPECT_TRUE(AF_INET == sa_default.get_family());

	// Construct from v4 addr, port (host byte order)
	bool is_valid = false;
	uint32 test_ip_lb = parse_ip(LOOPBACK_V4, &is_valid);
	EXPECT_TRUE(is_valid);
	sockaddr_construct_v4(test_ip_lb, TEST_PORT, AT_LOOPBACK);

	sockaddr_construct_v4(TEST_V4_ADDR_ANY, TEST_PORT, AT_ANY);

	is_valid = false;
	uint test_ip = parse_ip(ADDR_V4_MED, &is_valid);
	EXPECT_TRUE(is_valid);
	sockaddr_construct_v4(test_ip, TEST_PORT, AT_NORMAL);

	// Construct from SOCKADDR_STORAGE is done in storage test

	// Construct from string
	sockaddr_construct_v4(LOOPBACK_V4_WITH_TEST_PORT, test_ip_lb, TEST_PORT, AT_LOOPBACK);
	sockaddr_construct_v4(ANY_V4_WITH_TEST_PORT, TEST_V4_ADDR_ANY, TEST_PORT, AT_ANY);
	sockaddr_construct_v4(ADDR_V4_MED_WITH_TEST_PORT, test_ip, TEST_PORT, AT_NORMAL);

	// TODO:  Construct from "compact" allocation
	// SockAddr(const byte* p, size_t len, bool* success);
}

TEST(SockAddr, TestComparison)
{
	bool is_valid = false;

	uint32 test_ip_low = parse_ip(ADDR_V4_LOW, &is_valid);
	EXPECT_TRUE(is_valid);
	SockAddr sa_low(test_ip_low, TEST_PORT);
	EXPECT_TRUE(TEST_PORT == sa_low.get_port());
	EXPECT_TRUE(test_ip_low == sa_low.get_addr4());
	EXPECT_TRUE(!sa_low.is_loopback());
	EXPECT_TRUE(!sa_low.is_addr_any());

	uint32 test_ip_med = parse_ip(ADDR_V4_MED, &is_valid);
	EXPECT_TRUE(is_valid);
	SockAddr sa_med(test_ip_med, TEST_PORT);
	EXPECT_TRUE(TEST_PORT == sa_med.get_port());
	EXPECT_TRUE(test_ip_med == sa_med.get_addr4());
	EXPECT_TRUE(!sa_med.is_loopback());
	EXPECT_TRUE(!sa_med.is_addr_any());

	is_valid = false;
	uint32 test_ip_high = parse_ip(ADDR_V4_HIGH, &is_valid);
	EXPECT_TRUE(is_valid);
	SockAddr sa_high(test_ip_high, TEST_PORT);
	EXPECT_TRUE(TEST_PORT == sa_high.get_port());
	EXPECT_TRUE(test_ip_high == sa_high.get_addr4());
	EXPECT_TRUE(!sa_high.is_loopback());
	EXPECT_TRUE(!sa_high.is_addr_any());

	EXPECT_TRUE(sa_low == sa_low);
	EXPECT_TRUE(sa_med == sa_med);
	EXPECT_TRUE(sa_high == sa_high);

	EXPECT_TRUE(sa_med < sa_high);
	EXPECT_TRUE(sa_high > sa_med);
	EXPECT_TRUE(sa_med <= sa_high);
	EXPECT_TRUE(sa_high >= sa_med);
	EXPECT_TRUE(sa_med != sa_high);

	EXPECT_TRUE(sa_low < sa_high);
	EXPECT_TRUE(sa_high > sa_low);
	EXPECT_TRUE(sa_low <= sa_high);
	EXPECT_TRUE(sa_high >= sa_low);
	EXPECT_TRUE(sa_low != sa_high);

	EXPECT_TRUE(sa_low < sa_med);
	EXPECT_TRUE(sa_med > sa_low);
	EXPECT_TRUE(sa_low <= sa_med);
	EXPECT_TRUE(sa_med >= sa_low);
	EXPECT_TRUE(sa_low != sa_med);
}

TEST(SockAddr, TestStorage)
{
	// Set up the SockAddr from which to derive SOCKADDR_STORAGE
	bool is_valid = false;
	uint32 test_ip_a = parse_ip(ADDR_V4_MED, &is_valid);
	EXPECT_TRUE(is_valid);
	SockAddr sa_a(test_ip_a, TEST_PORT);
	EXPECT_TRUE(TEST_PORT == sa_a.get_port());
	EXPECT_TRUE(test_ip_a == sa_a.get_addr4());
	EXPECT_TRUE(!sa_a.is_loopback());
	EXPECT_TRUE(!sa_a.is_addr_any());

	// Test the retrieved SOCKADDR_STORAGE
	SOCKADDR_STORAGE sa;
	socklen_t len = 0;
	sa = sa_a.get_sockaddr_storage(&len);
	sockaddr_in& sain = (sockaddr_in&)sa;
	EXPECT_TRUE(sain.sin_addr.s_addr == htonl(test_ip_a));
	EXPECT_TRUE(sain.sin_port == htons(TEST_PORT));
	EXPECT_TRUE(sain.sin_family == AF_INET);

	// Construct a SockAddr from the SOCKADDR_STORAGE to test that constructor
	SockAddr sa_ss(sa);
	EXPECT_TRUE(TEST_PORT == sa_ss.get_port());
	EXPECT_TRUE(test_ip_a == sa_ss.get_addr4());
	EXPECT_TRUE(!sa_ss.is_loopback());
	EXPECT_TRUE(!sa_ss.is_addr_any());
}

TEST(SockAddr, parse_invalid_ipv6)
{
	bool valid = false;
	// pass in a string that's longer than 200 bytes (the internal buffer)
	SockAddr s = SockAddr::parse_addr(
		"[ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
		 "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff]", &valid);
	EXPECT_FALSE(valid);
	EXPECT_FALSE(true);
}

