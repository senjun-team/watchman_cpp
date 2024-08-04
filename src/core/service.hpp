#pragma once
#include "common/common.hpp"
#include "common/containers.hpp"

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
    Response runPlayground(RunCodeParams const & runCodeParams);

private:
    detail::ReleasingContainer getReadyContainer(Config::Config::ContainerType type);

    detail::ContainerController m_containerController;
};
}  // namespace watchman
