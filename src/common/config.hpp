#pragma once

#include "common/common.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace watchman {

struct ContainerTypeInfo {
    std::string imageName;
    uint32_t launched{0};
};

using Language = ContainerTypeInfo;
using Playground = ContainerTypeInfo;
using PracticeContainer = ContainerTypeInfo;

enum class TaskLauncherType {
    CPP_COURSE,
    CPP_PLAYGROUND,
    CPP_PRACTICE,
    GO_COURSE,
    GO_PLAYGROUND,
    GO_PRACTICE,
    HASKELL_COURSE,
    HASKELL_PLAYGROUND,
    HASKELL_PRACTICE,
    PYTHON_COURSE,
    PYTHON_PLAYGROUND,
    PYTHON_PRACTICE,
    RUST_COURSE,
    RUST_PLAYGROUND,
    RUST_PRACTICE,
    UNKNOWN
};

struct Config {
    using CodeLauncherType = TaskLauncherType;  // python/rust/go/haskell
    std::size_t threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<CodeLauncherType, Language> courses;
    std::unordered_map<CodeLauncherType, Playground> playgrounds;
    std::unordered_map<CodeLauncherType, PracticeContainer> practices;
};

std::optional<Config> getConfig();

// for tests
Config readConfig(std::string_view configPath);

TaskLauncherType constructTaskLauncher(std::string const & language, Api api);
}  // namespace watchman
