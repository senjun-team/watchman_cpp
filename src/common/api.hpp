#pragma once

#include <cstddef>
#include <cstdint>

namespace watchman {

enum class Api { Chapter, Playground, Practice };

enum class Language : uint8_t { CPP, GO, HASKELL, PYTHON, RUST };
using Action = Api;

struct LanguageAction {
    Language language;
    Action action;

    bool operator==(LanguageAction const & other) const {
        return language == other.language && action == other.action;
    }
};

struct LanguageActionHasher {
    size_t operator()(LanguageAction const & l) const {
        constexpr size_t kBitsInByte = 8;
        constexpr size_t kShift = sizeof(Language) * kBitsInByte;

        size_t seed = 0;
        seed ^= static_cast<size_t>(l.language);
        seed <<= kShift;
        seed ^= static_cast<size_t>(l.action);
        return seed;
    }
};

}  // namespace watchman
