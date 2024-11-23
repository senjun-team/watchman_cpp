#pragma once

#include "common/config.hpp"
#include "core/code_launcher/code_launcher_interface.hpp"

namespace watchman {

class CodeLauncherProviderInterface {
public:
    virtual ~CodeLauncherProviderInterface() = default;

    virtual std::unique_ptr<CodeLauncherInterface>
    getCodeLauncher(Config::CodeLauncherType const & type) = 0;
};

}  // namespace watchman
