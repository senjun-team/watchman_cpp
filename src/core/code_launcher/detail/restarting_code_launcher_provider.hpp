#pragma once

#include "common/common.hpp"
#include "core/code_launcher/code_launcher_provider_interface.hpp"

namespace watchman::detail {

class CodeLauncherOSManipulator;

class RestartingCodeLauncher : public CodeLauncherProviderInterface {
public:
    explicit RestartingCodeLauncher(Config && config);
    ~RestartingCodeLauncher();

    RestartingCodeLauncher(RestartingCodeLauncher const & other) = delete;
    RestartingCodeLauncher(RestartingCodeLauncher && other) = delete;
    RestartingCodeLauncher & operator=(RestartingCodeLauncher const & other) = delete;
    RestartingCodeLauncher & operator=(RestartingCodeLauncher && other) = delete;

    std::unique_ptr<CodeLauncherInterface>
    getCodeLauncher(Config::CodeLauncherType const & type) override;

private:
    void restartCodeLauncher(CodeLauncherInfo const & info);
    bool codeLauncherTypeIsValid(std::string const & name) const;

    ProtectedLaunchers m_protectedLaunchers;
    std::unique_ptr<CodeLauncherOSManipulator> m_manipulator;
};

}  // namespace watchman::detail
