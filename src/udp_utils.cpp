
#include "udp_utils.h"

uint16 GetUDP_MTU(const SockAddr& remote)
{
	// TODO: This function should look like this:
	/*
	const SockAddr local = GetInterfaceAddressForRoute(remote);
	if (local.isv6()) {
		assert(remote.isv6());
		if (IsTeredoAddress(local.get_addr6()) ||
			IsTeredoAddress(remote.get_addr6())) {
			return UDP_TEREDO_MTU;
		}
		return UDP_IPV6_MTU;
	}
	assert(local.isv4() && remote.isv4());
	return UDP_IPV4_MTU;
	*/

	// Instead, since we don't know the local address of the interface,
	// be conservative and assume all IPv6 connections are Teredo.
	// We should build GetInterfaceAddressForRoute using
	// pt2GetBestInterface/pt2GetBestInterfaceEx.
	return remote.isv6() ? UDP_TEREDO_MTU : UDP_IPV4_MTU;
}

uint16 GetUDP_Overhead(const SockAddr& remote)
{
	// TODO: This function should look like this:
	/*
	const SockAddr local = GetInterfaceAddressForRoute(remote);
	if (local.isv6()) {
		// Overhead is determined by our socket, not theirs. If they are Teredo
		// and we are not, there's some relay which is adding/stripping the overhead,
		// so we shouldn't count it.
		if (IsTeredoAddress(local.get_addr6()) {
			return UDP_TEREDO_OVERHEAD;
		}
		return UDP_IPV6_OVERHEAD;
	}
	assert(local.isv4());
	return UDP_IPV4_OVERHEAD;
	*/

	// Instead, since we don't know the local address of the interface,
	// be conservative and assume all IPv6 connections are Teredo.
	// We should build GetInterfaceAddressForRoute using
	// pt2GetBestInterface/pt2GetBestInterfaceEx.
	return remote.isv6() ? UDP_TEREDO_OVERHEAD : UDP_IPV4_OVERHEAD;
}

UDPSocketInterface::~UDPSocketInterface() {}

