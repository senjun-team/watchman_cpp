#pragma once

#include "common/common.hpp"
#include "docker_wrapper.hpp"

#include <condition_variable>
#include <docker/answer.hpp>
#include <functional>
#include <mutex>
#include <optional>
#include <unifex/single_thread_context.hpp>
#include <unordered_map>
#include <vector>

namespace watchman {
namespace detail {

class ContainerInterface {
public:
    virtual ~ContainerInterface() = default;

    virtual Response runCode(std::vector<std::string> && cmdLineArgs) = 0;
};

struct BaseContainer : ContainerInterface {
    DockerWrapper dockerWrapper;
    std::string id;
    Config::ContainerType type;
    bool isReserved{false};

    BaseContainer(std::string id, Config::ContainerType type);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::ostringstream && stream);
};

struct CourseContainer final : BaseContainer {
    CourseContainer(std::string id, Config::ContainerType type);
    Response runCode(std::vector<std::string> && cmdLineArgs) override;
};

struct PlaygroundContainer final : BaseContainer {
    PlaygroundContainer(std::string id, Config::ContainerType type);

    Response runCode(std::vector<std::string> && cmdLineArgs) override;
};

class ContainerController {
public:
    explicit ContainerController(Config && config);
    ~ContainerController();

    ContainerController(ContainerController const & other) = delete;
    ContainerController(ContainerController && other) = delete;
    ContainerController & operator=(ContainerController const & other) = delete;
    ContainerController & operator=(ContainerController && other) = delete;

    BaseContainer & getReadyContainer(Config::ContainerType const & type);
    void containerReleased(BaseContainer & container);
    bool containerNameIsValid(const std::string & name) const;

    void removeContainerFromOs(std::string const & id);

private:
    std::unordered_map<Config::ContainerType, std::vector<std::shared_ptr<BaseContainer>>>
        m_containers;

    std::mutex m_mutex;
    std::condition_variable m_containerFree;

    DockerWrapper m_dockerWrapper;
    unifex::single_thread_context m_containerKiller;

    Config m_config;

    void killOldContainers(DockerWrapper & dockerWrapper);
    void launchNewContainers(DockerWrapper & dockerWrapper);
};

class ReleasingContainer {
public:
    ReleasingContainer(BaseContainer & container, std::function<void()> deleter);
    ~ReleasingContainer();

    ReleasingContainer(ReleasingContainer const &) = delete;
    ReleasingContainer(ReleasingContainer &&) = delete;
    ReleasingContainer & operator=(ReleasingContainer const &) = delete;
    ReleasingContainer & operator=(ReleasingContainer &&) = delete;

    BaseContainer & container;

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
    Response runPlayground(RunCodeParams const & runCodeParams);

private:
    detail::ReleasingContainer getReadyContainer(Config::Config::ContainerType type);

    detail::ContainerController m_containerController;
};
}  // namespace watchman
