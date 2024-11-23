#pragma once

#include "common/common.hpp"
#include "core/code_launcher/code_launcher_interface.hpp"

namespace watchman::detail {

class CodeLauncherOSManipulator;

class CodeLauncherController {
public:
    explicit CodeLauncherController(Config && config);
    ~CodeLauncherController();

    CodeLauncherController(CodeLauncherController const & other) = delete;
    CodeLauncherController(CodeLauncherController && other) = delete;
    CodeLauncherController & operator=(CodeLauncherController const & other) = delete;
    CodeLauncherController & operator=(CodeLauncherController && other) = delete;

    std::unique_ptr<CodeLauncherInterface> getCodeLauncher(Config::CodeLauncherType const & type);
    void restartCodeLauncher(CodeLauncherInfo const & info);
    bool codeLauncherTypeIsValid(std::string const & name) const;

private:
    void removeCodeLauncher(std::string const & id);
    void createNewCodeLauncher(Config::CodeLauncherType type, std::string const & image);

    ProtectedLaunchers m_protectedLaunchers;
    std::unique_ptr<CodeLauncherOSManipulator> m_manipulator;
};


}  // namespace watchman::detail
