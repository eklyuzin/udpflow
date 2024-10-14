#pragma once

#include "ICommandHandler.h"

#include <memory>

namespace udpflow
{

class Sender;
class StatCollector;

class CommandHandler : public ICommandHandler
{
public:
	explicit CommandHandler(std::shared_ptr<Stat> stat);
	~CommandHandler() override;

public:
	void StartMeasure() override;
	void StopMeasure() override;

	void StartTransferTo(std::uint32_t destination_address, std::uint16_t port, std::uint64_t rate_limit = 0) override;
	void StopTransfer() override;

private:
	const std::shared_ptr<Stat> stat_;

	std::unique_ptr<Sender> sender_;
};

} // namespace udpflow
