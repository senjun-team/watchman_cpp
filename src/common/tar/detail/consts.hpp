#pragma once
#include "header.hpp"

namespace tar::detail {

constexpr auto kNullTerminator = '\0';
constexpr auto kNullCharacter = '0';

constexpr auto kTarHeaderSize = sizeof(detail::TarHeader);
}  // namespace tar::detail
