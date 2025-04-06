#pragma once

#include <cstddef>
#include <cstdint>

namespace watchman {

enum class Api { Chapter, Playground, Practice };

enum class Language : uint8_t { CPP, GO, HASKELL, PYTHON, RUST };
using Action = Api;

}  // namespace watchman
