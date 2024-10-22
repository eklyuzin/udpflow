#include "udpflow/Receiver.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <linux/in.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

namespace udpflow
{

Receiver::Receiver(Receiver::DataHandler data_handler, std::uint16_t port)
	: data_handler_{std::move(data_handler)}
	, sockfd_{socket(AF_INET, SOCK_DGRAM, 0)}
{
	if (!data_handler_)
		throw std::runtime_error("Invalid args");

	if (sockfd_ < 0)
		throw std::runtime_error{"Unable to create socket"};

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET; // IPv4
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	if (bind(sockfd_, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		close(sockfd_); // TODO: Use RAII
		throw std::runtime_error{"Bind failed"};
	}

	if (0 != pthread_create(&pthread_, nullptr, &Receiver::ThreadFunc, static_cast<void *>(this)))
	{
		close(sockfd_);
		throw std::runtime_error{"Unable to create thread"};
	}
}

Receiver::~Receiver()
{
	stop_flag_ = true;
	close(sockfd_);
	pthread_join(pthread_, 0);
}

void * Receiver::ThreadFunc(void * _self)
{
	const auto self = static_cast<Receiver *>(_self);

	char buffer[1 * 1024];
	struct sockaddr_in client_address;
	memset(&client_address, 0, sizeof(client_address));

	while (!self->stop_flag_)
	{
		socklen_t len = sizeof(client_address); // len is value/result
		const int n = recvfrom(
			self->sockfd_,

			(char *)buffer,
			sizeof(buffer),
			MSG_DONTWAIT,
			(struct sockaddr *)&client_address,
			&len);
		if (n < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Handle non-blocking receive error
				continue;
			}
			perror("recvfrom");
			break;
		}
		self->data_handler_(client_address, buffer, n);
	}
	return 0;
}

} // namespace udpflow
