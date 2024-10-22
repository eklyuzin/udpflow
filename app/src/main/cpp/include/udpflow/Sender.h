#pragma once

#include "udpflow/Consts.h"
#include "udpflow/Stat.h"

#include <cstdint>
#include <memory>
#include <pthread.h>
#include <string>

namespace udpflow
{

class Stat;

class Sender
{
public:
	static constexpr std::uint16_t default_port = constants::default_receive_port;
	static constexpr std::size_t default_send_times = 10;

public:
	Sender(std::shared_ptr<Stat> stat, std::uint32_t destination_address, std::uint16_t port = default_port);
	~Sender();

	static void Send(
		const std::string & message,
		const std::string & destination_address,
		std::uint16_t port = default_port,
		std::size_t times = default_send_times);

	static void Send(
		const std::string & message,
		std::uint32_t destination_address,
		std::uint16_t port = default_port,
		std::size_t times = default_send_times);

private:
	static void * ThreadFunc(void * _self);

private:
	const std::shared_ptr<Stat> stat_;
	const std::uint32_t destination_address_;
	const std::uint16_t port_;

	int sockfd_ = {};

	bool stop_flag_ = false;
	pthread_t pthread_;
};

} // namespace udpflow
