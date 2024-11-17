#pragma once

#include "common/common.hpp"
#include "core/docker_wrapper.hpp"

#include <functional>
#include <vector>

namespace watchman::detail {

class ContainerLauncherInterface {
public:
    virtual ~ContainerLauncherInterface() = default;

    virtual Response runCode(std::vector<std::string> && cmdLineArgs) = 0;
};

struct BaseContainerLauncher : ContainerLauncherInterface {
    DockerWrapper dockerWrapper;
    std::string id;
    Config::ContainerType type;
    bool isReserved{false};

    BaseContainerLauncher(std::string id, Config::ContainerType type);

    // Creates in-memory tar and passes it to docker
    bool prepareCode(std::string && tarString);
};

struct CourseContainerLauncher final : BaseContainerLauncher {
    CourseContainerLauncher(std::string id, Config::ContainerType type);
    Response runCode(std::vector<std::string> && cmdLineArgs) override;
};

struct PlaygroundContainerLauncher final : BaseContainerLauncher {
    PlaygroundContainerLauncher(std::string id, Config::ContainerType type);
    Response runCode(std::vector<std::string> && cmdLineArgs) override;
};

struct PracticeContainerLauncher final : BaseContainerLauncher {
    PracticeContainerLauncher(std::string id, Config::ContainerType type);
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

    BaseContainerLauncher & getReadyContainer(Config::ContainerType const & type);
    void containerReleased(BaseContainerLauncher & container);
    bool containerNameIsValid(const std::string & name) const;

private:
    void removeContainerFromOs(std::string const & id);
    void createNewContainer(Config::ContainerType type, std::string const & image);

    ProtectedContainers m_protectedContainers;
    std::unique_ptr<ContainerOSManipulator> m_manipulator;
};

class ReleasingContainer {
public:
    ReleasingContainer(BaseContainerLauncher & container, std::function<void()> deleter);
    ~ReleasingContainer();

    ReleasingContainer(ReleasingContainer const &) = delete;
    ReleasingContainer(ReleasingContainer &&) = delete;
    ReleasingContainer & operator=(ReleasingContainer const &) = delete;
    ReleasingContainer & operator=(ReleasingContainer &&) = delete;

    BaseContainerLauncher & container;

private:
    std::function<void()> m_releaser;
};

}  // namespace watchman::detail
