#include "udpflow/Server.h"

#include "udpflow/ICommandHandler.h"
#include "udpflow/Stat.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <linux/in.h>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

extern void output(const std::string & str);

namespace udpflow
{
namespace
{

void process_command(
	ICommandHandler & command_handler,
	const std::string & command_text,
	const sockaddr_in & client_address)
{
	output(std::string("Process command: ") + command_text);

	std::vector<std::string> tokens;
	std::stringstream ss(command_text);
	for (std::string str; getline(ss, str, ' ');)
		tokens.push_back(str);
	if (tokens.empty())
		return;
	const std::string_view cmd = tokens[0];
	using namespace std::literals::string_view_literals;
	if ("start"sv == cmd)
	{
		command_handler.StartMeasure();
	}
	else if ("stop"sv == cmd)
	{
		command_handler.StopMeasure();
	}
	else if ("start_send"sv == cmd)
	{
		// start_send [<ip> <port> <rate_limit>]
		command_handler.StartTransferTo(client_address.sin_addr.s_addr);
	}
	else if ("stop_send"sv == cmd)
	{
		command_handler.StopTransfer();
	}
	else if ("start_stat"sv == cmd)
	{
		command_handler.StartStat();
	}
	else if ("stop_stat"sv == cmd)
	{
		command_handler.StopStat();
	}
}

} // namespace

Server::Server(std::shared_ptr<ICommandHandler> command_handler, std::shared_ptr<Stat> stat, std::uint16_t port)
	: command_handler_{std::move(command_handler)}
	, stat_{std::move(stat)}
	, sockfd_{socket(AF_INET, SOCK_DGRAM, 0)}
{
	if (!command_handler_ || !stat_)
		throw std::runtime_error("Invalid args");

	if (sockfd_ < 0)
	{
		throw std::runtime_error{"Unable to create socket"};
	}

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

	if (0 != pthread_create(&pthread_, nullptr, &Server::ThreadFunc, static_cast<void *>(this)))
	{
		close(sockfd_);
		throw std::runtime_error{"Unable to create thread"};
	}
}

Server::~Server()
{
	stop_flag_ = true;
	close(sockfd_);
	pthread_join(pthread_, 0);
}

void * Server::ThreadFunc(void * _self)
{
	const auto self = static_cast<Server *>(_self);

	char buffer[64 * 1024];
	struct sockaddr_in client_address;
	decltype(client_address.sin_addr.s_addr) last_peer_addr = {};
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
		self->stat_->AddRecv(n);

		if (client_address.sin_addr.s_addr != last_peer_addr)
		{
			last_peer_addr = client_address.sin_addr.s_addr;
			char ip_str[INET_ADDRSTRLEN];
			output(
				"Receive from: "
				+ std::string(inet_ntop(AF_INET, &(client_address.sin_addr), &ip_str[0], INET_ADDRSTRLEN))
				+ std::string(" r: ") + std::to_string(n));
		}
		if (buffer + n
			== std::find_if_not(
				buffer,
				buffer + n,
				[](char c)
				{
					return std::isprint(c);
				}))
		{
			process_command(*self->command_handler_, std::string{buffer, static_cast<std::size_t>(n)}, client_address);
		}
	}
	return 0;
}

} // namespace udpflow
