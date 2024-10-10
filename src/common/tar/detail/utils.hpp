#pragma once

#include "common/tar/detail/header.hpp"
#include "consts.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace tar::detail {
auto const getString = [](auto && value, uint32_t size) -> std::string {
    std::stringstream ss;
    // use convertion to octal intentionally
    ss << std::oct << std::setw(static_cast<int>(size)) << std::setfill(kNullCharacter) << value;
    return ss.str();
};

auto const toArrayFromString = [](auto const & from, auto & toArray) -> void {
    std::copy(from.begin(), from.begin() + std::min(from.size(), sizeof(toArray) - 1),
              toArray.begin());
};

inline uint32_t calculateChecksum(Header const & header) {
    uint32_t checksumValue = 0;
    std::array<uint8_t, sizeof(Header)> const & firstBytePointer =
        *reinterpret_cast<std::array<uint8_t, sizeof(Header)> const *>(&header);

    for (uint32_t i = 0; i != detail::kTarHeaderSize; ++i) {
        checksumValue += firstBytePointer[i];
    }
    return checksumValue;
}

}  // namespace tar::detail
