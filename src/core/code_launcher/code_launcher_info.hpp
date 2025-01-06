#pragma once
#include "common/config.hpp"

namespace watchman {

struct CodeLauncherInfo {
    std::string containerId;
    std::string image;
    LanguageAction type;
};

}  // namespace watchman
