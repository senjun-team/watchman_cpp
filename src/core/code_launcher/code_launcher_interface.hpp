#pragma once

#include "core/code_launcher/code_launcher_info.hpp"
#include "core/code_launcher/response.hpp"

#include <string>
#include <vector>

namespace watchman {

class CodeLauncherInterface {
public:
    virtual ~CodeLauncherInterface() = default;

    virtual Response runCode(std::string && inMemoryTarWithSources,
                             std::vector<std::string> && cmdLineArgs, Action action) = 0;

    virtual CodeLauncherInfo getInfo() const = 0;
};

}  // namespace watchman
