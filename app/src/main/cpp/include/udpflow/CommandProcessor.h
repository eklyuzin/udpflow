#pragma once

#include <memory>
#include <string>

struct sockaddr_in;

namespace udpflow
{

class ICommandHandler;

class CommandProcessor
{
public:
	explicit CommandProcessor(std::shared_ptr<ICommandHandler> command_handler);

public:
	void process(const std::string & command_text, const sockaddr_in & client_address);

private:
	const std::shared_ptr<ICommandHandler> command_handler_;
};

} // namespace udpflow
