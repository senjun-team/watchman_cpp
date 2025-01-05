#pragma once

#include "common/config.hpp"
#include "core/code_launcher/code_launcher_interface.hpp"
#include "core/docker_wrapper.hpp"

namespace watchman {

struct BaseCodeLauncher : CodeLauncherInterface {
    DockerWrapper dockerWrapper;
    std::string containerId;
    TaskLauncherType type;

    BaseCodeLauncher(std::string id, TaskLauncherType type);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::string && tarString, Action type);
    CodeLauncherInfo getInfo() const override;
};

struct CourseCodeLauncher final : BaseCodeLauncher {
    CourseCodeLauncher(std::string id, TaskLauncherType type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;
};

struct PlaygroundCodeLauncher final : BaseCodeLauncher {
    PlaygroundCodeLauncher(std::string id, TaskLauncherType type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;
};

struct PracticeCodeLauncher final : BaseCodeLauncher {
    PracticeCodeLauncher(std::string id, TaskLauncherType type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && dockerCmdLineArgs) override;
};

}  // namespace watchman
