#pragma once

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

struct Config {
    using CodeLauncherType = std::string;  // python/rust/go/haskell
    std::optional<size_t> threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<CodeLauncherType, Language> languages;
    std::unordered_map<CodeLauncherType, Playground> playgrounds;
    std::unordered_map<CodeLauncherType, PracticeContainer> practices;
};

Config readConfig(std::string_view configPath);

}  // namespace watchman
