#include "udpflow/CommandHandler.h"

#include "Sender.h"
#include "udpflow/StatCollector.h"

namespace udpflow
{

CommandHandler::CommandHandler(std::shared_ptr<Stat> stat)
	: stat_{std::move(stat)}
{
}

CommandHandler::~CommandHandler() = default;

void CommandHandler::StartMeasure()
{
}

void CommandHandler::StopMeasure()
{
}

void CommandHandler::StartTransferTo(std::uint32_t destination_address, std::uint16_t port, std::uint64_t rate_limit)
{
	StopTransfer();
	sender_ = std::make_unique<Sender>(stat_, destination_address, port);
}

void CommandHandler::StopTransfer()
{
	sender_.reset();
}

} // namespace udpflow
