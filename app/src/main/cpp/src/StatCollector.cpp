#include "udpflow/StatCollector.h"

#include <sstream>

namespace udpflow
{

StatCollector::StatCollector(std::shared_ptr<Stat> stat)
	: stat_{std::move(stat)}
{
	if (!stat_)
		throw std::runtime_error("Invalid args");
}

void StatCollector::onTick()
{
	data_.emplace_back(std::chrono::steady_clock::now(), stat_->GetData());
}

std::string StatCollector::dump() const
{
	if (data_.size() < 2)
		return {};
	std::ostringstream oss;
	for (auto it = std::next(data_.begin()); it != data_.end(); ++it)
	{
		const auto & [_, prev_data] = *std::prev(it);
		const auto & [tp, data] = *it;
		oss << tp.time_since_epoch().count() << ',' << data.recv - prev_data.recv << ',' << data.sent - prev_data.sent
			<< '\n';
	}
	return oss.str();
}

} // namespace udpflow
