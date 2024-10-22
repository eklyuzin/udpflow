#pragma once

#include "Consts.h"

#include <cstdint>
#include <memory>

struct sockaddr_in;

namespace udpflow
{

class Receiver;
class CommandProcessor;
class Stat;

class Server
{
public:
	Server(
		std::shared_ptr<CommandProcessor> command_processor,
		std::shared_ptr<Stat> stat,
		std::uint16_t port = constants::default_receive_port);
	~Server();

private:
	void OnData(struct sockaddr_in & client_address, const char * data, int data_len);

private:
	const std::shared_ptr<CommandProcessor> command_processor_;
	const std::shared_ptr<Stat> stat_;
	const std::unique_ptr<Receiver> receiver_;

	std::uint32_t last_peer_address_ = {};
};

} // namespace udpflow
