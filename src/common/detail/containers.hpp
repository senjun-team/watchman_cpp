#pragma once

#include "common/common.hpp"
#include "core/docker_wrapper.hpp"

#include <functional>
#include <vector>

namespace watchman::detail {

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

struct PracticeContainer final : BaseContainer {
    PracticeContainer(std::string id, Config::ContainerType type);
    Response runCode(std::vector<std::string> && dockerCmdLineArgs) override;
};

class ContainerOSManipulator;
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
    void createNewContainer(Config::ContainerType type, std::string const & image);

private:
    ProtectedContainers m_protectedCcontainers;
    std::unique_ptr<ContainerOSManipulator> m_manipulator;

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

}  // namespace watchman::detail
