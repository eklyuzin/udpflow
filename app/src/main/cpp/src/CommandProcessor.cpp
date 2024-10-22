#include "udpflow/CommandProcessor.h"

#include "udpflow/External.h"
#include "udpflow/ICommandHandler.h"
#include "udpflow/Utils.h"

#include <netinet/in.h>
#include <sstream>
#include <vector>

namespace udpflow
{

CommandProcessor::CommandProcessor(std::shared_ptr<ICommandHandler> command_handler)
	: command_handler_{std::move(command_handler)}
{
}

void CommandProcessor::process(const std::string & command_text, const sockaddr_in & client_address)
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
		command_handler_->StartMeasure();
	}
	else if ("stop"sv == cmd)
	{
		command_handler_->StopMeasure();
	}
	else if ("start_send"sv == cmd)
	{
		// start_send [<ip> <port> <rate_limit>]
		const auto dest_address = (tokens.size() > 1) ? IpFromString(tokens[1]) : client_address.sin_addr.s_addr;
		command_handler_->StartTransferTo(dest_address);
	}
	else if ("stop_send"sv == cmd)
	{
		command_handler_->StopTransfer();
	}
	else if ("start_stat"sv == cmd)
	{
		command_handler_->StartStat();
	}
	else if ("stop_stat"sv == cmd)
	{
		command_handler_->StopStat();
	}
}

} // namespace udpflow