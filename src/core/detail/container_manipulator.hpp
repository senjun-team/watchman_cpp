#pragma once

#include "common/common.hpp"
#include "core/docker_wrapper.hpp"

#include <unifex/single_thread_context.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/then.hpp>

namespace watchman::detail {

class ContainerOSManipulator {
public:
    ContainerOSManipulator(Config && config, ProtectedLaunchers & protectedContainers);

    void asyncRemoveContainerFromOs(std::string const & id);
    void asyncCreateNewContainer(Config::CodeLauncherType type, std::string const & image);

private:
    void syncKillRunningContainers(Config const & config);
    void syncLaunchNewContainers(Config const & config);

    std::unique_ptr<BaseCodeLauncher> createContainer(std::string const & containerType,
                                                   std::string const & imageName);
    void killContainer(std::string const & id);

    ProtectedLaunchers & m_protectedContainers;

    unifex::single_thread_context m_containersContext;
    DockerWrapper m_dockerWrapper;
};

}  // namespace watchman::detail
