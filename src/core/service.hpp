#pragma once

#include "common/common.hpp"
#include "core/code_launcher/code_launcher_provider_interface.hpp"

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
    std::unique_ptr<CodeLauncherProviderInterface> m_codeLauncherProvider;
};
}  // namespace watchman
