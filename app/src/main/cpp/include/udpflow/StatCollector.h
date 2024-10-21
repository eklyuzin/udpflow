#pragma once

#include "Stat.h"

#include <chrono>
#include <deque>
#include <memory>
#include <utility>

namespace udpflow
{

class StatCollector
{
public:
	explicit StatCollector(std::shared_ptr<Stat> stat);
	void onTick();
	[[nodiscard]] std::string dump() const;

private:
	const std::shared_ptr<Stat> stat_;
	std::deque<std::pair<std::chrono::steady_clock::time_point, Stat::Data>> data_;
};

} // namespace udpflow
