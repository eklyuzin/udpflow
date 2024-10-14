#pragma once

#include "../../src/Sender.h"

#include <cstdint>

namespace udpflow
{

class ICommandHandler
{
public:
	virtual ~ICommandHandler() = default;
	virtual void StartMeasure() = 0;
	virtual void StopMeasure() = 0;

	virtual void StartTransferTo(
		std::uint32_t destination_address,
		std::uint16_t port = Sender::default_port,
		std::uint64_t rate_limit = 0) = 0;
	virtual void StopTransfer() = 0;

	virtual void StartStat() = 0;
	virtual void StopStat() = 0;

};

} // namespace udpflow
