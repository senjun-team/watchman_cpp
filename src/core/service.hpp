#pragma once

#include "common/common.hpp"
#include "docker_wrapper.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

namespace watchman {
namespace detail {

struct Container {
    DockerWrapper dockerWrapper;
    std::string id;
    Config::ContainerType type;
    bool isReserved{false};

    Container(std::string id, Config::ContainerType type);
    Response runCode(std::vector<std::string> && cmdLineArgs);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::string const & code, std::string const & codeTests);
};

class ContainerController {
public:
    explicit ContainerController(Config && config);
    ~ContainerController();

    ContainerController(ContainerController const & other) = delete;
    ContainerController(ContainerController && other) = delete;
    ContainerController & operator=(ContainerController const & other) = delete;
    ContainerController & operator=(ContainerController && other) = delete;

    Container & getReadyContainer(Config::ContainerType type);
    void containerReleased(Container & container);
    bool containerNameIsValid(const std::string & name)const;

private:
    std::unordered_map<Config::ContainerType, std::vector<std::shared_ptr<Container>>> m_containers;

    std::mutex m_mutex;
    std::condition_variable m_containerFree;

    Config m_config;

    void killOldContainers(DockerWrapper & dockerWrapper,
                           std::unordered_map<Config::ContainerType, Language> const & languages);
    void launchNewContainers(DockerWrapper & dockerWrapper,
                             std::unordered_map<Config::ContainerType, Language> const & languages);
};

class ReleasingContainer {
public:
    ReleasingContainer(Container & container, std::function<void()> deleter);
    ~ReleasingContainer();

    ReleasingContainer(ReleasingContainer const &) = delete;
    ReleasingContainer(ReleasingContainer &&) = delete;
    ReleasingContainer & operator=(ReleasingContainer const &) = delete;
    ReleasingContainer & operator=(ReleasingContainer &&) = delete;

    Container & container;

private:
    std::function<void()> m_releaser;
};

}  // namespace detail

class Service {
public:
    explicit Service(Config && config);
    ~Service() = default;

    Service(Service const &) = delete;
    Service(Service &&) = delete;

    Service & operator=(Service const &) = delete;
    Service & operator=(Service &&) = delete;

    Response runTask(RunTaskParams const & runTaskParams);

private:
    detail::ReleasingContainer getReadyContainer(Config::Config::ContainerType type);

    detail::ContainerController m_containerController;
};
}  // namespace watchman
