#include "udpflow/Server.h"

#include "udpflow/CommandProcessor.h"
#include "udpflow/External.h"
#include "udpflow/Receiver.h"
#include "udpflow/Stat.h"

#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>

namespace udpflow
{

Server::Server(std::shared_ptr<CommandProcessor> command_processor, std::shared_ptr<Stat> stat, std::uint16_t port)
	: command_processor_{std::move(command_processor)}
	, stat_{std::move(stat)}
	, receiver_{std::make_unique<Receiver>(
		  [this](struct sockaddr_in & client_address, const char * data, int data_len)
		  {
			  OnData(client_address, data, data_len);
		  },
		  port)}

{
	if (!command_processor_ || !stat_)
		throw std::runtime_error("Invalid args");
}

Server::~Server() = default;

void Server::OnData(struct sockaddr_in & client_address, const char * data, const int data_len)
{
	stat_->AddRecv(data_len);

	if (client_address.sin_addr.s_addr != last_peer_address_)
	{
		last_peer_address_ = client_address.sin_addr.s_addr;
		char ip_str[INET_ADDRSTRLEN];
		output(
			"Receive from: " + std::string(inet_ntop(AF_INET, &(client_address.sin_addr), &ip_str[0], INET_ADDRSTRLEN))
			+ std::string(" r: ") + std::to_string(data_len));
	}
	if (data + data_len
		== std::find_if_not(
			data,
			data + data_len,
			[](char c)
			{
				return std::isprint(c);
			}))
	{
		command_processor_->process(std::string{data, static_cast<std::size_t>(data_len)}, client_address);
	}
}

} // namespace udpflow
