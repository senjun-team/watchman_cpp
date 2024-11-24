#pragma once

#include "common/config.hpp"
#include "core/code_launcher/detail/code_launchers.hpp"
#include "core/code_launcher/detail/storage.hpp"
#include "core/code_launcher/response.hpp"
#include "core/docker_wrapper.hpp"

#include <unifex/single_thread_context.hpp>

namespace watchman::detail {

using Storage = ExtractingStorage<Config::CodeLauncherType, BaseCodeLauncher>;

class CodeLauncherOSManipulator {
public:
    CodeLauncherOSManipulator(Config && config, Storage & storage);

    void asyncRemoveCodeLauncher(std::string const & id);
    void asyncCreateCodeLauncher(Config::CodeLauncherType type, std::string const & image);

private:
    void syncRemoveRunningCodeLanchers(Config const & config);
    void syncCreateCodeLaunchers(Config const & config);

    // todo
    // user pair, not code launcher info
    std::unique_ptr<BaseCodeLauncher> createCodeLauncher(CodeLauncherInfo const & info);
    void removeCodeLauncher(std::string const & id);

    // Original storage lies in restarting_code_launcher.hpp
    Storage & m_storage;

    unifex::single_thread_context m_containersContext;
    DockerWrapper m_dockerWrapper;
};

}  // namespace watchman::detail
