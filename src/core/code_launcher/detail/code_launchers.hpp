#pragma once

#include "core/code_launcher/code_launcher_interface.hpp"
#include "core/docker_wrapper.hpp"

namespace watchman {

struct BaseCodeLauncher : CodeLauncherInterface {
    DockerWrapper dockerWrapper;
    std::string containerId;
    Language type;

    BaseCodeLauncher(std::string id, Language language);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::string && tarString, Action type);

    Response runCode(std::string && inMemoryTarWithSources, std::vector<std::string> && cmdLineArgs,
                     Action action) override;

    CodeLauncherInfo getInfo() const override;
};

}  // namespace watchman
