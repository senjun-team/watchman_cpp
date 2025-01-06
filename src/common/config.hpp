#pragma once

#include "common/common.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace watchman {

struct TaskLauncherInfo {
    std::string imageName;
    uint32_t launched{0};  // TODO useless field. We start containers at beginning once
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

struct Config {
    std::size_t threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<LanguageAction, TaskLauncherInfo, LanguageActionHasher> courses;
    std::unordered_map<LanguageAction, TaskLauncherInfo, LanguageActionHasher> playgrounds;
    std::unordered_map<LanguageAction, TaskLauncherInfo, LanguageActionHasher> practices;
};

std::optional<Config> getConfig();

// for tests
Config readConfig(std::string_view configPath);

LanguageAction constructTaskLauncher(std::string const & language, Api api);
}  // namespace watchman
