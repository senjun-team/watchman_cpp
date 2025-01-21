#pragma once

#include "common/api.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace watchman {

struct ContainerInfo {
    std::string imageName;
    uint32_t launched{0};
};

using ConfigContainerStorage = std::unordered_map<Language, ContainerInfo>;

struct Config {
    std::size_t threadPoolSize;
    uint32_t maxContainersAmount{0};

    ConfigContainerStorage courses;
    ConfigContainerStorage playgrounds;
    ConfigContainerStorage practices;
};

std::optional<Config> getConfig();

// for tests
Config readConfig(std::string_view configPath);

Language getLanguageFromString(std::string const & language);
}  // namespace watchman
