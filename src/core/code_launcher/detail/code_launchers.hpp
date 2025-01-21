#pragma once

#include "core/code_launcher/code_launcher_interface.hpp"
#include "core/docker_wrapper.hpp"

namespace watchman {

struct BaseCodeLauncher : CodeLauncherInterface {
    DockerWrapper dockerWrapper;
    std::string containerId;
    Language type;

    BaseCodeLauncher(std::string id, Language type);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::string && tarString, Action type);
};

struct ChapterCodeLauncher final : BaseCodeLauncher {
    ChapterCodeLauncher(std::string id, Language type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;
    CodeLauncherInfo getInfo() const override;
};

struct PlaygroundCodeLauncher final : BaseCodeLauncher {
    PlaygroundCodeLauncher(std::string id, Language type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;
    CodeLauncherInfo getInfo() const override;
};

struct PracticeCodeLauncher final : BaseCodeLauncher {
    PracticeCodeLauncher(std::string id, Language type);
    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && dockerCmdLineArgs) override;
    CodeLauncherInfo getInfo() const override;
};

}  // namespace watchman
