#pragma once

#include "common/common.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace watchman {

struct TaskLauncherInfo {
    std::string imageName;
    uint32_t launched{0};
};

enum class ImageType { Task, Playground, Practice };

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
    std::size_t threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<TaskLauncherType, TaskLauncherInfo> courses;
    std::unordered_map<TaskLauncherType, TaskLauncherInfo> playgrounds;
    std::unordered_map<TaskLauncherType, TaskLauncherInfo> practices;
};

std::optional<Config> getConfig();

// for tests
Config readConfig(std::string_view configPath);

TaskLauncherType constructTaskLauncher(std::string const & language, Api api);
}  // namespace watchman
