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
    : m_dockerHost(std::move(host))
    , m_config(configPath) {
    Log::info("Service launched");

    DockerWrapper dockerWrapper(m_dockerHost);
    killOldContainers(dockerWrapper, m_config.getLanguages());
    launchNewContainers(dockerWrapper, m_config.getLanguages());
}

detail::Container & detail::ContainerController::getReadyContainer(detail::Container::Type type) {
    std::unique_lock lock(m_mutex);

    auto & containers = m_containers.at(type);
    size_t indexProperContainer;  // there's no need for initialization

    m_containerFree.wait(lock, [&containers, &indexProperContainer]() -> bool {
        for (size_t index = 0; index < containers.size(); ++index) {
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

void detail::ContainerController::containerReleased(Container & container) {
    {
        std::scoped_lock lock(m_mutex);
        container.isReserved = false;
    }
    m_containerFree.notify_one();
}

detail::ContainerController::~ContainerController() {}

void detail::ContainerController::killOldContainers(
    DockerWrapper & dockerWrapper,
    std::unordered_map<Container::Type, Language> const & languages) {
    auto const workingContainers = dockerWrapper.getAllContainers();
    for (auto const & [_, language] : languages) {
        for (auto const & container : workingContainers) {
            if (container.image.find(language.imageName) != std::string::npos) {
                if (dockerWrapper.isRunning(container.id)) {
                    dockerWrapper.killContainer(container.id);
                }

                Log::info("Remove container type: {}, id: {}", language.imageName, container.id);
                dockerWrapper.removeContainer(container.id);
            }
        }
    }
}

void detail::ContainerController::launchNewContainers(
    DockerWrapper & dockerWrapper,
    std::unordered_map<Container::Type, Language> const & languages) {
    for (auto const & [type, language] : languages) {
        std::vector<std::shared_ptr<Container>> containers;
        for (size_t index = 0; index < language.launched; ++index) {
            DockerRunParams params{
                .image = language.imageName, .tty = true, .memoryLimit = 300'000'000};
            std::string id = dockerWrapper.run(std::move(params));
            if (id.empty()) {
                Log::warning("Internal error: can't run container of type {}",
                             static_cast<int>(type));
                continue;
            }

            Log::info("Launch container: {}, id: {}", language.imageName, id);
            auto container = std::make_shared<Container>(m_dockerHost, std::move(id), type);
            containers.emplace_back(std::move(container));
        }

        if (!containers.empty()) {
            m_containers.emplace(type, std::move(containers));
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
                                      runTaskParams.containerType),
                .testsOutput = ""};
    }

    auto & container = getReadyContainer(containerType);  // here we have got a race

    auto resultSourceRun = container.runCode(runTaskParams.sourceRun);
    if (!resultSourceRun.isValid()) {
        m_containerController.containerReleased(container);

        return {.sourceCode = resultSourceRun.code,
                .testsCode = kInvalidCode,
                .output = std::move(resultSourceRun.output),
                .testsOutput = ""};
    }

    std::string testResult;
    if (runTaskParams.sourceTest.has_value() && !runTaskParams.sourceTest->empty()) {
        auto result = container.runCode(runTaskParams.sourceTest.value());
        testResult = std::move(result.output);

        if (!result.isValid()) {
            m_containerController.containerReleased(container);

            return {.sourceCode = kSuccessCode,
                    .testsCode = result.code,
                    .output = std::move(resultSourceRun.output),
                    .testsOutput = std::move(testResult)};
        }
    }

    if (auto result = container.clean(); !result.isValid()) {
        m_containerController.containerReleased(container);

        Log::error("Error while removing files for {}", container.id);
        return {.sourceCode = kSuccessCode,
                .testsCode = kSuccessCode,
                .output = std::move(resultSourceRun.output),
                .testsOutput = std::move(result.output)};
    }

    m_containerController.containerReleased(container);
    return {kSuccessCode, kSuccessCode, resultSourceRun.output, testResult};
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
    std::string const archive = id + "archiveWatchman.tar";
    if (!makeTar(archive, code)) {
        Log::error("Internal error! Couldn't create an archive");
        return {kInvalidCode, fmt::format("Internal error! Couldn't create an archive")};
    }

    if (!dockerWrapper.putArchive({id, kUserSourceFile, archive})) {
        Log::error("Internal error! Couldn't put an archive to container");
        return {kInvalidCode, fmt::format("Internal error! Couldn't put an archive to container")};
    }

    std::vector<std::string> const args{"sh", "run.sh", id + "archiveWatchman"};
    auto result = dockerWrapper.exec({args, id});
    if (result.exitCode < 0) {
        return {result.exitCode, result.output};
    }

    // TODO linux and mac differences with \r\n in answer
    auto const exitStatus = result.output[result.output.size() - 1] - '0';  // get status as integer
    result.output.resize(result.output.size() - 1);  // for linux there's no \r\n, only exitStatus

    if ((std::remove(archive.c_str())) != 0) {
        // TODO is this an error? should we write something to resul.output?
        Log::error("Internal error! Couldn't remove an archive from host");
    }

    return {exitStatus, result.output};
}

detail::Container::DockerAnswer detail::Container::clean() {
    // TODO may be here we need to remove user code from container or in runCode
    return {.code = kSuccessCode};
}

detail::Container::Container(std::string host, std::string id, Type type)
    : dockerWrapper(std::move(host))
    , id(std::move(id))
    , type(type) {}

bool detail::Container::DockerAnswer::isValid() const { return code == kSuccessCode; }

detail::ConfigParser::ConfigParser(std::string const & configPath) {
    namespace pt = boost::property_tree;
    pt::ptree loadPtreeRoot;

    try {
        pt::read_json(configPath, loadPtreeRoot);
    } catch (std::exception const & error) {
        Log::error("Error while reading config file: {}", error.what());
        std::terminate();
    }

    fillConfig(loadPtreeRoot);
}

std::unordered_map<detail::Container::Type, detail::Language>
detail::ConfigParser::getLanguages() const {
    return m_config.languages;
}

template<typename Ptree>
void detail::ConfigParser::fillConfig(Ptree const & root) {
    auto const & maxContainersAmount = root.get_child_optional("max-containers-amount");
    if (!maxContainersAmount.has_value()) {
        Log::error("Required field \'max-containers-amount\' is absent");
        std::terminate();
    }
    m_config.maxContainersAmount = maxContainersAmount.value().template get_value<uint32_t>();

    auto const & languages = root.get_child_optional("languages");
    if (!languages.has_value()) {
        Log::error("Required field \'languages\' is absent");
        std::terminate();
    }

    for (auto const & language : languages.value()) {
        auto imageName = language.second.get_child_optional("image-name");
        if (!imageName.has_value()) {
            Log::error("Required field \'image-name\' is absent in {}", language.first);
            std::terminate();
        }

        auto const launched = language.second.get_child_optional("launched");
        if (!launched.has_value()) {
            Log::error("Required field \'launched\' is absent in {}", language.first);
            std::terminate();
        }

        auto const containerType = getContainerType(language.first);
        if (containerType == Container::Type::Unknown) {
            Log::error("Container type: \'{}\' is unknown", language.first);
            std::terminate();
        }

        m_config.languages.insert({containerType,
                                   {imageName.value().template get_value<std::string>(),
                                    launched.value().template get_value<uint32_t>()}});
    }
}

}  // namespace watchman
