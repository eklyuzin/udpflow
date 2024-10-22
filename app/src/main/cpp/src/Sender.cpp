#include "udpflow/Sender.h"

#include "udpflow/External.h"
#include "udpflow/Stat.h"
#include "udpflow/Utils.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace udpflow
{

Sender::Sender(std::shared_ptr<Stat> stat, std::uint32_t destination_address, std::uint16_t port)
	: stat_{std::move(stat)}
	, destination_address_{destination_address}
	, port_{port}
	, sockfd_{socket(AF_INET, SOCK_DGRAM, 0)}
{
	if (!stat_)
		throw std::runtime_error("Invalid args");

	if (sockfd_ < 0)
		throw std::runtime_error{"Unable to create socket"};

	if (0 != pthread_create(&pthread_, nullptr, &Sender::ThreadFunc, static_cast<void *>(this)))
	{
		close(sockfd_);
		throw std::runtime_error{"Unable to create thread"};
	}
}

Sender::~Sender()
{
	stop_flag_ = true;
	close(sockfd_);
	pthread_join(pthread_, 0);
}

void * Sender::ThreadFunc(void * _self)
{
	const auto self = static_cast<Sender *>(_self);

	struct sockaddr_in destination;
	memset(&destination, 0, sizeof(destination));

	destination.sin_family = AF_INET; // IPv4
	destination.sin_addr.s_addr = self->destination_address_;
	destination.sin_port = htons(self->port_);

	char ip_str[INET_ADDRSTRLEN];
	output(
		"Send to: " + std::string(inet_ntop(AF_INET, &(self->destination_address_), &ip_str[0], INET_ADDRSTRLEN)) + ":"
		+ std::to_string(self->port_));

	std::uint64_t c = 0;
	while (!self->stop_flag_)
	{
		std::uint64_t buffer[1500 / sizeof(c)];
		for (int i = 0; i < sizeof(buffer) / sizeof(*buffer); ++i)
			buffer[i] = c++;
		int n = sendto(
			self->sockfd_,
			(const void *)buffer,
			sizeof(buffer),
			0,
			(sockaddr *)&destination,
			sizeof(destination));
		if (n < 0)
		{
			perror("sendto");
			break;
		}
		self->stat_->AddSent(n);
	}
	return 0;
}

void Sender::Send(const std::string & message, std::uint32_t destination_address, std::uint16_t port, std::size_t times)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in destination;
	memset(&destination, 0, sizeof(destination));

	destination.sin_family = AF_INET; // IPv4
	destination.sin_addr.s_addr = destination_address;
	destination.sin_port = htons(port);

	do
	{
		int n = sendto(
			sockfd,
			(const void *)message.c_str(),
			message.size(),
			0,
			(sockaddr *)&destination,
			sizeof(destination));
		if (n < 0)
		{
			perror("sendto");
		}
	}
	while (--times);
}
void Sender::Send(
	const std::string & message,
	const std::string & destination_address,
	std::uint16_t port,
	std::size_t times)
{
	Send(message, IpFromString(destination_address), port, times);
}

} // namespace udpflow
