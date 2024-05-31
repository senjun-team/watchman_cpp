#include "service.hpp"

#include "common/docker_end_answer.hpp"
#include "common/logging.hpp"
#include "core/docker_wrapper.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <cctype>
#include <future>
#include <string_view>

namespace watchman {

struct StringImages {
    static std::string_view constexpr python = "senjun_courses_python";
    static std::string_view constexpr rust = "senjun_courses_rust";
};

static std::string const kUserSourceFile = "/home/code_runner";
static std::string const kUserSourceFileTests = "/home/code_runner";

Service::Service(Config && config)
    : m_containerController(std::move(config)) {}

detail::ContainerController::ContainerController(Config && config)
    : m_config(std::move(config)) {
    Log::info("Service launched");

    DockerWrapper dockerWrapper;
    killOldContainers(dockerWrapper);
    launchNewContainers(dockerWrapper);
}

detail::BaseContainer &
detail::ContainerController::getReadyContainer(Config::ContainerType const & type) {
    // lock for any container type
    // todo think of separating containers into different queues
    std::unique_lock lock(m_mutex);

    auto & containers = m_containers.at(type);
    size_t indexProperContainer;  // there's no need for initialization

    m_containerFree.wait(lock, [&containers, &indexProperContainer]() -> bool {
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

void detail::ContainerController::containerReleased(BaseContainer & container) {
    {
        std::scoped_lock const lock(m_mutex);
        container.isReserved = false;
    }
    m_containerFree.notify_one();
}

detail::ContainerController::~ContainerController() = default;

void detail::ContainerController::killOldContainers(DockerWrapper & dockerWrapper) {
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
}

void detail::ContainerController::launchNewContainers(DockerWrapper & dockerWrapper) {
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
                } else {
                    container = std::make_shared<CourseContainer>(std::move(id), type);
                }

                containers.emplace_back(std::move(container));
            }

            if (!containers.empty()) {
                m_containers.emplace(type, std::move(containers));
            }
        }
    };

    launchContainers(m_config.languages);
    launchContainers(m_config.playgrounds);
}

// Returns vector containing sequence: cmd, script, filename, args
std::vector<std::string> getArgs(std::string const & filename,
                                 std::vector<std::string> const & cmdLineArgs) {
    std::string const cmd = "sh";
    std::string const script = "run.sh";

    std::vector<std::string> runArgs;

    runArgs.reserve(3 + cmdLineArgs.size());

    runArgs.emplace_back(cmd);
    runArgs.emplace_back(script);
    runArgs.emplace_back(fmt::format("-f {}", filename));

    runArgs.insert(runArgs.end(), cmdLineArgs.begin(), cmdLineArgs.end());
    return runArgs;
}

Response watchman::Service::runTask(watchman::RunTaskParams const & runTaskParams) {
    if (runTaskParams.sourceRun.empty() && runTaskParams.sourceTest.empty()) {
        Log::warning("Empty files with code and test");
        return {kUserCodeError, "Sources and tests are not provided", ""};
    }

    if (!m_containerController.containerNameIsValid(runTaskParams.containerType)) {
        Log::warning("Invalid container type: {}", runTaskParams.containerType);
        return {kInvalidCode,
                fmt::format("Invalid container type: {}", runTaskParams.containerType),
                std::string{}};
    }

    auto raiiContainer = getReadyContainer(runTaskParams.containerType);  // here we have got a race
    auto & container = raiiContainer.container;

    std::vector<CodeFilename> data{{runTaskParams.sourceRun, kFilenameTask},
                                   {runTaskParams.sourceTest, kFilenameTaskTests}};

    if (!container.prepareCode(makeTar(std::move(data)))) {
        Log::warning("Couldn't pass tar to container. Source: {}", runTaskParams.sourceRun);
        return {};
    }

    auto result = container.runCode(getArgs(kFilenameTask, runTaskParams.cmdLineArgs));
    if (errorCodeIsUnexpected(result.sourceCode)) {
        Log::error("Error return code {} from container {} type of {}", result.sourceCode,
                   container.id, container.type);
    }
    return result;
}

Response Service::runPlayground(RunCodeParams const & runCodeParams) {
    if (runCodeParams.sourceRun.empty()) {
        Log::warning("Empty file with code");
        return {kUserCodeError, "Source code not provided"};
    }

    if (!m_containerController.containerNameIsValid(runCodeParams.containerType)) {
        Log::warning("Invalid container type: {}", runCodeParams.containerType);
        return {};
    }

    auto raiiContainer = getReadyContainer(runCodeParams.containerType);  // here we have got a race
    auto & container = raiiContainer.container;

    std::vector<CodeFilename> data{{runCodeParams.sourceRun, kFilenameTask}};
    if (!container.prepareCode(makeTar(std::move(data)))) {
        Log::warning("Couldn't pass tar to container. Source: {}", runCodeParams.sourceRun);
        return {};
    }

    return container.runCode(getArgs(kFilenameTask, runCodeParams.cmdLineArgs));
}

detail::ReleasingContainer Service::getReadyContainer(Config::ContainerType type) {
    auto & container = m_containerController.getReadyContainer(type);

    return {container,
            [this, &container]() { m_containerController.containerReleased(container); }};
}

bool detail::BaseContainer::prepareCode(std::ostringstream && stream) {
    PutArchive params;
    params.containerId = id;
    params.path = kUserSourceFile;
    params.archive = std::move(stream.str());

    if (!dockerWrapper.putArchive(std::move(params))) {
        Log::error("Couldn't put an archive to container");
        return false;
    }

    return true;
}

detail::PlaygroundContainer::PlaygroundContainer(std::string id, Config::ContainerType type)
    : BaseContainer(std::move(id), type) {}

Response detail::CourseContainer::runCode(std::vector<std::string> && cmdLineArgs) {
    auto result = dockerWrapper.exec({.containerId = id, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getCourseResponse(result.message);
}

detail::BaseContainer::BaseContainer(std::string id, Config::ContainerType type)
    : id(std::move(id))
    , type(std::move(type)) {}

detail::CourseContainer::CourseContainer(std::string id, Config::ContainerType type)
    : BaseContainer(std::move(id), type) {}

detail::ReleasingContainer::ReleasingContainer(detail::BaseContainer & container,
                                               std::function<void()> deleter)
    : container(container)
    , m_releaser(std::move(deleter)) {}

detail::ReleasingContainer::~ReleasingContainer() { m_releaser(); }

bool detail::ContainerController::containerNameIsValid(const std::string & name) const {
    return m_containers.contains(name);
}

Response detail::PlaygroundContainer::runCode(std::vector<std::string> && cmdLineArgs) {
    auto result = dockerWrapper.exec({.containerId = id, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getPlaygroungResponse(result.message);
}
}  // namespace watchman
