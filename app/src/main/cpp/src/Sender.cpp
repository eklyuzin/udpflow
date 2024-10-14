#include "Sender.h"

#include "udpflow/Stat.h"

#include <arpa/inet.h>
//#include <linux/in.h>
#include <sys/socket.h>
#include <unistd.h>

extern void output(const std::string & str);

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

	while (!self->stop_flag_)
	{
		char buffer[1024];
		int n = sendto(
			self->sockfd_	,
			(const void *)buffer,
			sizeof(buffer),
			0,
			(sockaddr *)&destination,
			sizeof(destination));
		self->stat_->AddSent(n);
	}
	return 0;
}

} // namespace udpflow
