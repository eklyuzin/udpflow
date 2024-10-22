#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace udpflow
{

std::vector<std::string> GetIps();

std::uint32_t IpFromString(const std::string & ip);

void OutputIPv4Addresses();

} // namespace udpflow
