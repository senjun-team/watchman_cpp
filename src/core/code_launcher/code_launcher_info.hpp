#pragma once
#include "common/api.hpp"

#include <string>

namespace watchman {

struct CodeLauncherInfo {
    std::string containerId;
    std::string image;
    LanguageAction type;
};

}  // namespace watchman
