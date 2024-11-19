#pragma once

#include "common/common.hpp"
#include "core/code_launcher/code_launchers.hpp"

#include <functional>
#include <vector>

namespace watchman::detail {

class ContainerOSManipulator;

class CodeLauncherController {
public:
    explicit CodeLauncherController(Config && config);
    ~CodeLauncherController();

    CodeLauncherController(CodeLauncherController const & other) = delete;
    CodeLauncherController(CodeLauncherController && other) = delete;
    CodeLauncherController & operator=(CodeLauncherController const & other) = delete;
    CodeLauncherController & operator=(CodeLauncherController && other) = delete;

    std::unique_ptr<BaseCodeLauncher> getCodeLauncher(Config::CodeLauncherType const & type);
    void restartCodeLauncher(LauncherRestartInfo const & restartInfo);
    bool launcherTypeIsValid(std::string const & name) const;

private:
    void removeContainerFromOs(std::string const & id);
    void createNewContainer(Config::CodeLauncherType type, std::string const & image);

    ProtectedLaunchers m_protectedLaunchers;
    std::unique_ptr<ContainerOSManipulator> m_manipulator;
};

class RestartingLauncher {
public:
    RestartingLauncher(std::unique_ptr<BaseCodeLauncher> launcher, std::function<void()> deleter);
    ~RestartingLauncher();

    RestartingLauncher(RestartingLauncher const &) = delete;
    RestartingLauncher(RestartingLauncher &&) = delete;
    RestartingLauncher & operator=(RestartingLauncher const &) = delete;
    RestartingLauncher & operator=(RestartingLauncher &&) = delete;

    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs);

private:
    std::unique_ptr<BaseCodeLauncher> m_codeLauncher;
    std::function<void()> m_releaser;
};

}  // namespace watchman::detail
