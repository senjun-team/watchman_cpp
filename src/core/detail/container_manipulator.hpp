#pragma once

#include "common/common.hpp"
#include "core/docker_wrapper.hpp"

#include <unifex/single_thread_context.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/then.hpp>

namespace watchman::detail {

class ContainerOSManipulator {
public:
    ContainerOSManipulator(Config && config, ProtectedContainers & protectedContainers);

    void asyncRemoveContainerFromOs(std::string const & id);
    void asyncCreateNewContainer(Config::ContainerType type, std::string const & image);

private:
    void syncKillRunningContainers(Config const & config);
    void syncLaunchNewContainers(Config const & config);

    ProtectedContainers & m_protectedContainers;

    unifex::single_thread_context m_containerKillerAliver;
    DockerWrapper m_dockerWrapper;
};

}  // namespace watchman::detail
