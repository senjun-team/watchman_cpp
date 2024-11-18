#pragma once

#include "common/common.hpp"
#include "core/docker_wrapper.hpp"

#include <functional>
#include <vector>

namespace watchman::detail {

class CodeLauncherInterface {
public:
    virtual ~CodeLauncherInterface() = default;

    virtual Response runCode(std::string && inMemoryTarWithSources,
                             std::vector<std::string> && cmdLineArgs) = 0;
};

struct LauncherRestartInfo {
    std::string containerId;
    std::string image;
    std::string containerType;
};

struct BaseCodeLauncher : CodeLauncherInterface {
    DockerWrapper dockerWrapper;
    std::string containerId;
    Config::CodeLauncherType type;

    BaseCodeLauncher(std::string id, Config::CodeLauncherType type);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::string && tarString);
    LauncherRestartInfo getRestartInfo() const;
};

struct CourseCodeLauncher final : BaseCodeLauncher {
    CourseCodeLauncher(std::string id, Config::CodeLauncherType type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;
};

struct PlaygroundCodeLauncher final : BaseCodeLauncher {
    PlaygroundCodeLauncher(std::string id, Config::CodeLauncherType type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;
};

struct PracticeCodeLauncher final : BaseCodeLauncher {
    PracticeCodeLauncher(std::string id, Config::CodeLauncherType type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && dockerCmdLineArgs) override;
};

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

    std::unique_ptr<BaseCodeLauncher> codeLauncher;

private:
    std::function<void()> m_releaser;
};

}  // namespace watchman::detail
