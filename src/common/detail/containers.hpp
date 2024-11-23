#pragma once

#include "common/common.hpp"
#include "core/code_launcher/code_launcher_interface.hpp"
#include "core/code_launcher/code_launchers.hpp"

#include <functional>
#include <vector>

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

class RestartingCodeLauncher : public CodeLauncherInterface {
public:
    RestartingCodeLauncher(std::unique_ptr<CodeLauncherInterface> launcher,
                           std::function<void()> deleter);
    ~RestartingCodeLauncher();

    RestartingCodeLauncher(RestartingCodeLauncher const &) = delete;
    RestartingCodeLauncher(RestartingCodeLauncher &&) = delete;
    RestartingCodeLauncher & operator=(RestartingCodeLauncher const &) = delete;
    RestartingCodeLauncher & operator=(RestartingCodeLauncher &&) = delete;

    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;

    
    CodeLauncherInfo getInfo() const override;

private:
    std::unique_ptr<CodeLauncherInterface> m_codeLauncher;
    std::function<void()> m_releaser;
};

}  // namespace watchman::detail
