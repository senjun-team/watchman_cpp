#pragma once

#include "core/code_launcher/code_launcher_interface.hpp"
#include "core/code_launcher/detail/containers.hpp"

namespace watchman {

class Service {
public:
    explicit Service(Config && config);
    ~Service() = default;

    Service(Service const &) = delete;
    Service(Service &&) = delete;

    Service & operator=(Service const &) = delete;
    Service & operator=(Service &&) = delete;

    Response runTask(RunTaskParams const & runTaskParams);
    Response runPlayground(RunProjectParams const & runProjectParams);
    Response runPractice(RunPracticeParams const & params);

private:
    std::unique_ptr<CodeLauncherInterface> getCodeLauncher(Config::CodeLauncherType type);
    detail::CodeLauncherController m_codeLauncherController;
};
}  // namespace watchman
