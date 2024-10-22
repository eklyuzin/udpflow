#include "udpflow/Utils.h"

#include "udpflow/External.h"

#include <arpa/inet.h>
#include <linux/netdevice.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace udpflow
{

std::vector<std::string> GetIps()
{
	const int domain = AF_INET;
	int s;
	struct ifconf ifconf;
	struct ifreq ifr[50];
	int ifs;
	int i;

	s = socket(domain, SOCK_STREAM, 0);
	if (s < 0)
	{
		perror("socket");
		return {};
	}

	ifconf.ifc_buf = (char *)ifr;
	ifconf.ifc_len = sizeof ifr;

	if (ioctl(s, SIOCGIFCONF, &ifconf) == -1)
	{
		perror("ioctl");
		return {};
	}

	ifs = ifconf.ifc_len / sizeof(ifr[0]);
	std::vector<std::string> ips;
	ips.reserve(ifs);
	for (i = 0; i < ifs; i++)
	{
		char ip[INET_ADDRSTRLEN];
		struct sockaddr_in * s_in = (struct sockaddr_in *)&ifr[i].ifr_addr;

		if (!inet_ntop(domain, &s_in->sin_addr, ip, sizeof(ip)))
		{
			perror("inet_ntop");
			continue;
		}

		ips.emplace_back(ip);
	}

	close(s);
	return ips;
}

void OutputIPv4Addresses()
{
	for (const auto & ip : GetIps())
		output(ip);
}

std::uint32_t IpFromString(const std::string & ip)
{
	struct sockaddr_in sa;
	inet_aton(ip.c_str(), &(sa.sin_addr));
	return sa.sin_addr.s_addr;
}

} // namespace udpflow