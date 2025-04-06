#pragma once

#include "core/code_launcher/code_launcher_interface.hpp"

#include <memory>

namespace watchman {

class CodeLauncherProviderInterface {
public:
    virtual ~CodeLauncherProviderInterface() = default;

    virtual std::unique_ptr<CodeLauncherInterface> getCodeLauncher(Language language) = 0;
};

}  // namespace watchman
