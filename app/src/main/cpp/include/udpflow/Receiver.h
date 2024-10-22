#pragma once

#include "Consts.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <pthread.h>

struct sockaddr_in;

namespace udpflow
{

class Receiver
{
public:
	static constexpr std::uint16_t default_port = constants::default_receive_port;

	using DataHandler = std::function<void(struct sockaddr_in & clint_address, const char * data, int data_len)>;

public:
	explicit Receiver(DataHandler data_handler, std::uint16_t port = default_port);
	~Receiver();

private:
	static void * ThreadFunc(void * _self);

private:
	const DataHandler data_handler_;

	int sockfd_ = {};

	bool stop_flag_ = false;
	pthread_t pthread_;
};

} // namespace udpflow
