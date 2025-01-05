#pragma once

#include "common/config.hpp"
#include "core/code_launcher/detail/code_launchers.hpp"
#include "core/code_launcher/detail/storage.hpp"
#include "core/docker_wrapper.hpp"

#include <unifex/single_thread_context.hpp>

namespace watchman::detail {

using Storage = ExtractingStorage<TaskLauncherType, BaseCodeLauncher>;

class CodeLauncherOSManipulator {
public:
    CodeLauncherOSManipulator(Config && config, Storage & storage);

    void asyncRemoveCodeLauncher(std::string const & id);
    void asyncCreateCodeLauncher(TaskLauncherType type, std::string const & image);

private:
    void syncRemoveRunningCodeLanchers();
    void syncCreateCodeLaunchers(Config const & config);

    // todo
    // user pair, not code launcher info
    std::unique_ptr<BaseCodeLauncher>
    createCodeLauncher(std::string const & image, TaskLauncherType taskType, Action imageType);
    void removeCodeLauncher(std::string const & id);

    // Original storage lies in restarting_code_launcher.hpp
    Storage & m_storage;

    unifex::single_thread_context m_containersContext;
    DockerWrapper m_dockerWrapper;
};

}  // namespace watchman::detail
