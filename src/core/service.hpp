#pragma once

#include "common/detail/containers.hpp"

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
    detail::RestartingLauncher getCodeLauncher(Config::CodeLauncherType type);
    detail::CodeLauncherController m_codeLauncherController;
};
}  // namespace watchman
