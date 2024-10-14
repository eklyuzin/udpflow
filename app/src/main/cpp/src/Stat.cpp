#include "udpflow/Stat.h"

#include <sstream>

namespace udpflow
{
void Stat::AddRecv(std::uint64_t n)
{
	const std::lock_guard lock(mutex_);
	data_.recv += n;
}

std::string Stat::GetStat() const
{
	const auto now = std::chrono::steady_clock::now();
	Data data;
	decltype(prev_data_) prev_data;
	{
		const std::lock_guard lock(mutex_);
		data = data_;
		prev_data = std::exchange(prev_data_, decltype(prev_data_)::value_type{now, data_});
	}
	if (prev_data)
	{
		std::ostringstream oss;
		oss << "Recv: "
			<< (data.recv - prev_data->second.recv)
				/ std::chrono::duration_cast<std::chrono::seconds>(now - prev_data->first).count()
			<< " Send: "
			<< (data.sent - prev_data->second.sent)
				/ std::chrono::duration_cast<std::chrono::seconds>(now - prev_data->first).count();
		return oss.str();
	}
	return std::string();
}

void Stat::AddSent(std::uint64_t n)
{
	const std::lock_guard lock(mutex_);
	data_.sent += n;
}

Stat::Data Stat::GetData() const
{
	const std::lock_guard lock(mutex_);
	return data_;
}

} // namespace udpflow
