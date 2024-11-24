#pragma once

#include "core/code_launcher/detail/storage.hpp"
#include "core/code_launcher/response.hpp"
#include "core/docker_wrapper.hpp"

#include <unifex/single_thread_context.hpp>

namespace watchman::detail {

class CodeLauncherOSManipulator {
public:
    CodeLauncherOSManipulator(Config && config, CodeLaunchersStorage & storage);

    void asyncRemoveCodeLauncher(std::string const & id);
    void asyncCreateCodeLauncher(Config::CodeLauncherType type, std::string const & image);

private:
    void syncRemoveRunningCodeLanchers(Config const & config);
    void syncCreateCodeLaunchers(Config const & config);

    // todo
    // user pair, not code launcher info
    std::unique_ptr<BaseCodeLauncher> createCodeLauncher(CodeLauncherInfo const & info);
    void removeCodeLauncher(std::string const & id);

    CodeLaunchersStorage & m_storage;

    unifex::single_thread_context m_containersContext;
    DockerWrapper m_dockerWrapper;
};

}  // namespace watchman::detail
