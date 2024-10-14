#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace udpflow
{
class Stat
{
public:
	struct Data
	{
		std::uint32_t recv{};
		std::uint32_t sent{};
	};
public:
	void AddRecv(std::uint64_t n);
	void AddSent(std::uint64_t n);

	Data GetData() const;
	std::string GetStat() const;

private:
	Data data_;
	mutable std::optional<std::pair<std::chrono::steady_clock::time_point, Data>> prev_data_;
	mutable std::mutex mutex_;
};

} // namespace udpflow
