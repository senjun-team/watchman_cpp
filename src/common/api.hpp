#pragma once

#include <string>

namespace watchman {

enum class Api { ChapterTask, Playground, Practice };

struct ContainerInfo {
    std::string imageName;
    uint32_t launched{0};
};

enum class Language : uint8_t { CPP, GO, HASKELL, PYTHON, RUST };
enum class Action : uint8_t { ChapterTask, Playground, Practice };

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
