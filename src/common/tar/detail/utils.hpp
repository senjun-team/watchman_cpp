#pragma once
#include "consts.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace tar::detail {
auto const getString = [](auto && value, uint32_t size) -> std::string {
    std::stringstream ss;
    // use convertion to octal intentionally
    ss << std::oct << std::setw(static_cast<int>(size)) << std::setfill('0') << value;
    return ss.str();
};

auto const toArrayFromString = [](auto const & from, auto & toArray) -> void {
    std::copy(from.begin(), from.begin() + std::min(from.size(), sizeof(toArray) - 1),
              toArray.begin());
};

inline uint32_t calculateChecksum(TarHeader const & header) {
    uint32_t checksumValue = 0;
    uint8_t const * const firstBytePointer = reinterpret_cast<uint8_t const * const>(&header);

    for (uint32_t i = 0; i != detail::kTarHeaderSize; ++i) {
        checksumValue += firstBytePointer[i];
    }
    return checksumValue;
}

}  // namespace tar::detail
