#pragma once

#include <cstdint>
#include <memory>
#include <pthread.h>

namespace udpflow
{

class ICommandHandler;
class Stat;

class Server
{
public:
	static constexpr std::uint16_t default_port = 1777;

public:
	Server(
		std::shared_ptr<ICommandHandler> command_handler,
		std::shared_ptr<Stat> stat,
		std::uint16_t port = default_port);
	~Server();

private:
	static void * ThreadFunc(void * _self);

private:
	const std::shared_ptr<ICommandHandler> command_handler_;
	const std::shared_ptr<Stat> stat_;

	int sockfd_ = {};

	bool stop_flag_ = false;
	pthread_t pthread_;
};

} // namespace udpflow
