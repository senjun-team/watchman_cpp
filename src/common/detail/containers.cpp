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
    : m_manipulator(std::make_unique<ContainerOSManipulator>(m_protectedCcontainers))
    , m_config(std::move(config)) {
    Log::info("Service launched");

    DockerWrapper dockerWrapper;
    killOldContainers(dockerWrapper);
    launchNewContainers(dockerWrapper);
}

BaseContainer & ContainerController::getReadyContainer(Config::ContainerType const & type) {
    // lock for any container type
    // todo think of separating containers into different queues
    std::unique_lock lock(m_protectedCcontainers.mutex);

    auto & containers = m_protectedCcontainers.containers.at(type);
    size_t indexProperContainer;  // there's no need for initialization

    m_protectedCcontainers.containerFree.wait(lock, [&containers, &indexProperContainer]() -> bool {
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
        std::scoped_lock const lock(m_protectedCcontainers.mutex);

        auto & containers = m_protectedCcontainers.containers.at(container.type);

        containers.erase(
            std::remove_if(containers.begin(), containers.end(),
                           [&container](auto const & c) { return c->id == container.id; }),
            containers.end());
    }
    removeContainerFromOs(id);
    createNewContainer(type, image);
}

ContainerController::~ContainerController() = default;

void ContainerController::killOldContainers(DockerWrapper & dockerWrapper) {
    auto const workingContainers = dockerWrapper.getAllContainers();

    // todo think about naming, containerTypes is awful
    auto const killContainers = [this, &dockerWrapper, &workingContainers](auto && containerTypes) {
        for (auto const & [_, info] : containerTypes) {
            for (auto const & container : workingContainers) {
                if (container.image.find(info.imageName) != std::string::npos) {
                    if (dockerWrapper.isRunning(container.id)) {
                        dockerWrapper.killContainer(container.id);
                    }

                    Log::info("Remove container type: {}, id: {}", info.imageName, container.id);
                    dockerWrapper.removeContainer(container.id);
                }
            }
        }
    };

    killContainers(m_config.languages);
    killContainers(m_config.playgrounds);
    killContainers(m_config.practices);
}

void ContainerController::launchNewContainers(DockerWrapper & dockerWrapper) {
    auto const launchContainers = [this, &dockerWrapper](auto && containerTypes) {
        for (auto const & [type, info] : containerTypes) {
            std::vector<std::shared_ptr<BaseContainer>> containers;
            for (size_t index = 0; index < info.launched; ++index) {
                RunContainer params;

                params.image = info.imageName;
                params.tty = true;
                params.memory = 300'000'000;

                std::string id = dockerWrapper.run(std::move(params));
                if (id.empty()) {
                    Log::warning("Internal error: can't run container of type {}", type);
                    continue;
                }

                Log::info("Launch container: {}, id: {}", info.imageName, id);

                std::shared_ptr<BaseContainer> container;
                if (info.imageName.find("playground") != std::string::npos) {
                    container = std::make_shared<PlaygroundContainer>(std::move(id), type);
                } else if (info.imageName.find("course") != std::string::npos) {
                    container = std::make_shared<CourseContainer>(std::move(id), type);
                } else if (info.imageName.find("practice") != std::string::npos) {
                    container = std::make_shared<PracticeContainer>(std::move(id), type);
                } else {
                    throw std::logic_error{"Wrong image name: " + info.imageName};
                }

                containers.emplace_back(std::move(container));
            }

            if (!containers.empty()) {
                m_protectedCcontainers.containers.emplace(type, std::move(containers));
            }
        }
    };

    launchContainers(m_config.languages);
    launchContainers(m_config.playgrounds);
    launchContainers(m_config.practices);
}

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
    return m_protectedCcontainers.containers.contains(name);
}

void ContainerController::removeContainerFromOs(std::string const & id) {
    m_manipulator->removeContainerFromOs(id);
}

void ContainerController::createNewContainer(Config::ContainerType type,
                                             std::string const & image) {
    m_manipulator->createNewContainer(type, image);
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
