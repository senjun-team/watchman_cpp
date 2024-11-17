#include "containers.hpp"

#include "common/detail/docker_end_answer.hpp"
#include "common/logging.hpp"
#include "core/detail/container_manipulator.hpp"

namespace watchman::detail {

static std::string const kUserSourceFile = "/home/code_runner";

BaseContainer::BaseContainer(std::string id, Config::ContainerType type)
    : id(std::move(id))
    , type(std::move(type)) {}

bool BaseContainer::prepareCode(std::string && tarString) {
    PutArchive params;
    params.containerId = id;
    params.path = kUserSourceFile;
    params.archive = std::move(tarString);
    params.copyUIDGID = "1";

    if (!dockerWrapper.putArchive(std::move(params))) {
        Log::error("Couldn't put an archive to container");
        return false;
    }

    return true;
}

Response PracticeContainer::runCode(std::vector<std::string> && dockerCmdLineArgs) {
    auto result = dockerWrapper.exec({.containerId = id, .cmd = std::move(dockerCmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getPracticeResponse(result.message);
}

ContainerController::ContainerController(Config && config)
    : m_manipulator(
          std::make_unique<ContainerOSManipulator>(std::move(config), m_protectedContainers)) {
    Log::info("Service launched");
}

BaseContainer & ContainerController::getReadyContainer(Config::ContainerType const & type) {
    // lock for any container type
    // todo think of separating containers into different queues
    std::unique_lock lock(m_protectedContainers.mutex);

    auto & containers = m_protectedContainers.containers.at(type);
    size_t indexProperContainer;  // there's no need for initialization

    m_protectedContainers.containerFree.wait(lock, [&containers, &indexProperContainer]() -> bool {
        for (size_t index = 0, size = containers.size(); index < size; ++index) {
            if (!containers[index]->isReserved) {
                indexProperContainer = index;
                containers[index]->isReserved = true;
                return true;
            }
        }
        return false;
    });

    return *containers.at(indexProperContainer);
}

void ContainerController::containerReleased(BaseContainer & container) {
    std::string const id = container.id;
    std::string const image = container.dockerWrapper.getImage(id);
    Config::ContainerType const type = container.type;
    {
        std::scoped_lock const lock(m_protectedContainers.mutex);

        auto & containers = m_protectedContainers.containers.at(container.type);

        containers.erase(
            std::remove_if(containers.begin(), containers.end(),
                           [&container](auto const & c) { return c->id == container.id; }),
            containers.end());
    }
    removeContainerFromOs(id);
    createNewContainer(type, image);
}

ContainerController::~ContainerController() = default;

PlaygroundContainer::PlaygroundContainer(std::string id, Config::ContainerType type)
    : BaseContainer(std::move(id), type) {}

Response CourseContainer::runCode(std::vector<std::string> && cmdLineArgs) {
    auto result = dockerWrapper.exec({.containerId = id, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getCourseResponse(result.message);
}

CourseContainer::CourseContainer(std::string id, Config::ContainerType type)
    : BaseContainer(std::move(id), type) {}

ReleasingContainer::ReleasingContainer(BaseContainer & container, std::function<void()> deleter)
    : container(container)
    , m_releaser(std::move(deleter)) {}

ReleasingContainer::~ReleasingContainer() { m_releaser(); }

bool ContainerController::containerNameIsValid(const std::string & name) const {
    return m_protectedContainers.containers.contains(name);
}

void ContainerController::removeContainerFromOs(std::string const & id) {
    m_manipulator->asyncRemoveContainerFromOs(id);
}

void ContainerController::createNewContainer(Config::ContainerType type,
                                             std::string const & image) {
    m_manipulator->asyncCreateNewContainer(type, image);
}

Response PlaygroundContainer::runCode(std::vector<std::string> && cmdLineArgs) {
    auto result = dockerWrapper.exec({.containerId = id, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getPlaygroungResponse(result.message);
}

PracticeContainer::PracticeContainer(std::string id, Config::ContainerType type)
    : BaseContainer(std::move(id), std::move(type)) {}

}  // namespace watchman::detail
