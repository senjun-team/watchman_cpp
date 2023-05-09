#pragma once

#include "common/common.hpp"
#include "docker_wrapper.hpp"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

namespace watchman {
namespace detail {

struct Container {

    struct DockerAnswer {
        ErrorCode code{kInvalidCode};
        std::string output;

        bool isValid() const;
    };

    DockerWrapper dockerWrapper;
    std::string id;
    ContainerType type{ContainerType::Unknown};
    bool isReserved{false};

    Container(std::string host, std::string id, ContainerType type);
    DockerAnswer runCode(std::string const & filename);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::string const & code, std::string const & codeTests);
    DockerAnswer clean();
};

class ContainerController {
public:
    ContainerController(std::string host, Config && config);
    ~ContainerController();

    ContainerController(ContainerController const & other) = delete;
    ContainerController(ContainerController && other) = delete;
    ContainerController & operator=(ContainerController const & other) = delete;
    ContainerController & operator=(ContainerController && other) = delete;

    Container & getReadyContainer(ContainerType type);
    void containerReleased(Container & container);

private:
    std::unordered_map<ContainerType, std::vector<std::shared_ptr<Container>>> m_containers;

    std::mutex m_mutex;
    std::condition_variable m_containerFree;

    Config m_config;
    std::string m_dockerHost;

    void killOldContainers(DockerWrapper & dockerWrapper,
                           std::unordered_map<ContainerType, Language> const & languages);
    void launchNewContainers(DockerWrapper & dockerWrapper,
                             std::unordered_map<ContainerType, Language> const & languages);
};
}  // namespace detail

class Service {
public:
    Service(std::string const & host, Config && config);
    ~Service() = default;

    Service(Service const &) = delete;
    Service(Service &&) = delete;

    Service & operator=(Service const &) = delete;
    Service & operator=(Service &&) = delete;

    Response runTask(RunTaskParams const & runTaskParams);

private:
    detail::Container & getReadyContainer(ContainerType type);

    detail::ContainerController m_containerController;
};
}  // namespace watchman
