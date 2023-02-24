#include "service.hpp"

#include "common/logging.hpp"

#include <boost/property_tree/json_parser.hpp>

namespace watchman {

struct StringContainers {
    static std::string_view constexpr python = "python";
    static std::string_view constexpr rust = "rust";
};

struct StringImages {
    static std::string_view constexpr python = "senjun_courses_python";
    static std::string_view constexpr rust = "senjun_courses_rust";
};

size_t constexpr kDockerTimeout = 124;
size_t constexpr kDockerMemoryKill = 137;

static std::string const kUserSourceFile = "/home/code_runner";
static std::string const kUserSourceFileTests = "/home/code_runner";

detail::Container::Type getContainerType(std::string const & type) {
    if (type == StringContainers::python) {
        return detail::Container::Type::Python;
    }

    if (type == StringContainers::rust) {
        return detail::Container::Type::Rust;
    }

    return detail::Container::Type::Unknown;
}

Service::Service(std::string const & host, std::string const & configPath)
    : m_containerController(host, configPath) {}

detail::ContainerController::ContainerController(std::string host, std::string const & configPath)
    : m_dockerWrapper(std::move(host))
    , m_maxContainersAmount(readConfig(configPath)) {}

detail::Container & detail::ContainerController::getReadyContainer(detail::Container::Type type) {
    std::unique_lock lock(m_mutex);

    auto & containers = m_containers.at(type);
    size_t indexProperContainer;  // there's no need for initialization

    m_containerFree.wait(lock, [&containers, &indexProperContainer]() -> bool {
        for (size_t index = 0; index < containers.size(); ++index) {
            if (!containers[index].isReserved) {
                indexProperContainer = index;
                containers[index].isReserved = true;
                return true;
            }
        }
        return false;
    });

    return containers.at(indexProperContainer);
}

void detail::ContainerController::containerReleased(Container & container) {
    {
        std::scoped_lock lock(m_mutex);
        container.isReserved = false;
    }
    m_containerFree.notify_one();
}

detail::ContainerController::~ContainerController() {
    for (auto & [_, containers] : m_containers) {
        for (auto & container : containers) {
            m_dockerWrapper.killContainer(container.id);
            m_dockerWrapper.removeContainer(container.id);
        }
    }
}

size_t detail::ContainerController::readConfig(std::string const & configPath) {
    namespace pt = boost::property_tree;
    pt::ptree loadPtreeRoot;

    try {
        pt::read_json(configPath, loadPtreeRoot);
    } catch (std::exception const & error) {
        Log::error("Error while reading config file: {}", error.what());
        std::terminate();
    }

    auto const & languages = loadPtreeRoot.get_child("languages");
    killOldContainers(languages);
    launchNewContainers(languages);

    return loadPtreeRoot.get_child("max-containers-amount").get_value<size_t>();
}

void detail::ContainerController::killOldContainers(boost::property_tree::ptree const & languages) {
    auto const workingContainers = m_dockerWrapper.getAllContainers();
    for (auto const & language : languages) {
        for (auto const & container : workingContainers) {
            if (container.image.find(
                    language.second.get_child("image-name").get_value<std::string>())
                != std::string::npos) {
                if (m_dockerWrapper.isRunning(container.id)) {
                    m_dockerWrapper.killContainer(container.id);
                }

                m_dockerWrapper.removeContainer(container.id);
            }
        }
    }
}

void detail::ContainerController::launchNewContainers(
    boost::property_tree::ptree const & languages) {
    for (auto const & language : languages) {
        auto const containerType = getContainerType(language.first);
        auto const imageName = language.second.get_child("image-name").get_value<std::string>();
        auto const alwaysLaunched = language.second.get_child("launched").get_value<size_t>();

        std::vector<Container> containers;
        containers.reserve(alwaysLaunched);
        for (size_t index = 0; index < alwaysLaunched; ++index) {
            DockerRunParams params{.image = imageName, .tty = true, .memoryLimit = 7000000};
            std::string id = m_dockerWrapper.run(std::move(params));
            if (id.empty()) {
                Log::warning("Internal error: can't run container of type {}",
                             static_cast<int>(containerType));
                continue;
            }
            containers.emplace_back(m_dockerWrapper, std::move(id), containerType);
        }

        if (!containers.empty()) {
            m_containers.emplace(containerType, std::move(containers));
        }
    }
}

Response watchman::Service::runTask(watchman::RunTaskParams const & runTaskParams) {
    auto const containerType = getContainerType(runTaskParams.containerType);
    if (containerType == detail::Container::Type::Unknown) {
        Log::warning("Unknown container type is provided");
        return {.sourceCode = kInvalidCode,
                .testsCode = kInvalidCode,
                .output = fmt::format("Error: unknown container type \'{}\'",
                                      runTaskParams.containerType)};
    }

    auto & container = getReadyContainer(containerType);  // here we have got a race

    auto resultSourceRun = container.runCode(runTaskParams.sourceRun);
    if (!resultSourceRun.isValid()) {
        return {.sourceCode = resultSourceRun.code,
                .testsCode = kInvalidCode,
                .output = std::move(resultSourceRun.output)};
    }

    if (runTaskParams.sourceTest.has_value() && !runTaskParams.sourceTest->empty()) {
        if (auto result = container.runCode(runTaskParams.sourceTest.value()); !result.isValid()) {
            return {.sourceCode = kSuccessCode,
                    .testsCode = result.code,
                    .output = std::move(result.output)};
        }
    }

    if (auto result = container.clean(); !result.isValid()) {
        Log::error("Error while removing files for {}", container.id);
        return {.sourceCode = kSuccessCode,
                .testsCode = kSuccessCode,
                .output = std::move(result.output)};
    }

    m_containerController.containerReleased(container);
    return {kSuccessCode, kSuccessCode, resultSourceRun.output};
}

detail::Container & Service::getReadyContainer(detail::Container::Type type) {
    return m_containerController.getReadyContainer(type);
}

detail::Container::Type Service::getContainerType(std::string const & type) {
    if (type == StringContainers::python) {
        return detail::Container::Type::Python;
    }

    if (type == StringContainers::rust) {
        return detail::Container::Type::Rust;
    }

    return detail::Container::Type::Unknown;
}

detail::Container::DockerAnswer detail::Container::runCode(std::string const & code) {
    // TODO rethink naming
    std::string const archive = "archiveWatchman.tar";
    if (!makeTar(archive, code)) {
        Log::error("Internal error! Couldn't create an archive");
        return {kInvalidCode, fmt::format("Internal error! Couldn't create an archive")};
    }

    if (!dockerWrapper.putArchive({id, kUserSourceFile, archive})) {
        Log::error("Internal error! Couldn't put an archive to container");
        return {kInvalidCode, fmt::format("Internal error! Couldn't put an archive to container")};
    }

    std::vector<std::string> const args{"sh", "run.sh", "archiveWatchman"};
    auto result = dockerWrapper.exec({args, id});

    if ((std::remove(archive.c_str())) != 0) {
        // TODO is this an error? should we write something to resul.output?
        Log::error("Internal error! Couldn't remove an archive from host");
    }

    return {result.exitCode, result.output};
}

detail::Container::DockerAnswer detail::Container::clean() {
    // TODO may be here we need to remove user code from container or in runCode
    return {.code = kSuccessCode};
}

detail::Container::Container(DockerWrapper & dockerWrapper, std::string id, Type type)
    : dockerWrapper(dockerWrapper)
    , id(std::move(id))
    , type(type) {}

bool detail::Container::DockerAnswer::isValid() const { return code == kSuccessCode; }

}  // namespace watchman
