#include "service.hpp"

#include "common/logging.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <map>
#include <string_view>

namespace watchman {
// 3 bytes: code\r\n
constexpr size_t kEscapeSequenceLen = 3;

// For all exit statuses of 'timeout' util see "man timeout"
static const std::map<std::string_view, int32_t> kTimeoutUtilCodes{
    {"124\r\n", 124},  // if COMMAND times out, and --preserve-status is not specified
    {"125\r\n", 125},  // if the timeout command itself fails
    {"137\r\n", 137}   // if COMMAND (or timeout itself) is sent the KILL (9)
};

// Also exit statuses of timeout util inside container:
size_t constexpr kDockerTimeout = 124;
size_t constexpr kDockerMemoryKill = 137;

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
    killOldContainers(dockerWrapper, m_config.languages);
    launchNewContainers(dockerWrapper, m_config.languages);
}

detail::Container & detail::ContainerController::getReadyContainer(ContainerType type) {
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
        std::scoped_lock const lock(m_mutex);
        container.isReserved = false;
    }
    m_containerFree.notify_one();
}

detail::ContainerController::~ContainerController() = default;

void detail::ContainerController::killOldContainers(
    DockerWrapper & dockerWrapper, std::unordered_map<ContainerType, Language> const & languages) {
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
    DockerWrapper & dockerWrapper, std::unordered_map<ContainerType, Language> const & languages) {
    for (auto const & [type, language] : languages) {
        std::vector<std::shared_ptr<Container>> containers;
        for (size_t index = 0; index < language.launched; ++index) {
            RunContainer params;

            params.image = language.imageName;
            params.tty = true;
            params.memory = 300'000'000;

            std::string id = dockerWrapper.run(std::move(params));
            if (id.empty()) {
                Log::warning("Internal error: can't run container of type {}",
                             static_cast<int>(type));
                continue;
            }

            Log::info("Launch container: {}, id: {}", language.imageName, id);
            auto container = std::make_shared<Container>(std::move(id), type);
            containers.emplace_back(std::move(container));
        }

        if (!containers.empty()) {
            m_containers.emplace(type, std::move(containers));
        }
    }
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
    auto const containerType = getContainerType(runTaskParams.containerType);
    if (containerType == ContainerType::Unknown) {
        Log::warning("Unknown container type is provided");
        return {.sourceCode = kInvalidCode,
                .testsCode = kInvalidCode,
                .output = fmt::format("Error: unknown container type \'{}\'",
                                      runTaskParams.containerType),
                .testsOutput = ""};
    }

    auto & container = getReadyContainer(containerType);  // here we have got a race
    if (!container.prepareCode(runTaskParams.sourceRun, runTaskParams.sourceTest)) {
        Log::warning("Couldn't pass tar to container");
        return {.sourceCode = kInvalidCode,
                .testsCode = kInvalidCode,
                .output = fmt::format("Couldn't pass tar to container"),
                .testsOutput = ""};
    }

    auto resultSourceRun = container.runCode(getArgs(kFilenameTask, runTaskParams.cmdLineArgs));
    if (!resultSourceRun.isValid()) {
        m_containerController.containerReleased(container);

        return {.sourceCode = resultSourceRun.code,
                .testsCode = kInvalidCode,
                .output = std::move(resultSourceRun.output),
                .testsOutput = ""};
    }

    std::string testResult;
    if (!runTaskParams.sourceTest.empty()) {
        auto result = container.runCode(getArgs(kFilenameTaskTests, runTaskParams.cmdLineArgs));
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

detail::Container & Service::getReadyContainer(ContainerType type) {
    return m_containerController.getReadyContainer(type);
}

bool detail::Container::prepareCode(std::string const & code, std::string const & codeTests) {
    std::ostringstream stream(std::ios::binary);

    stream = makeTar(code, codeTests);

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

bool findEscapeSequences(std::string const & output) {
    if (output.size() < kEscapeSequenceLen) {
        return false;
    }

    return output[output.size() - 1] == '\n' && output[output.size() - 2] == '\r';
}

int32_t getExitCode(bool hasEscapeSequences, std::string & output) {
    // Case when timeout util returns its status code
    for (auto const & [codeStr, code] : kTimeoutUtilCodes) {
        if (output.ends_with(codeStr)) {
            output.resize(output.size() - codeStr.size());
            return code;
        }
    }

    int32_t exitStatus = -1;

    if (hasEscapeSequences) {
        exitStatus = output[output.size() - kEscapeSequenceLen] - '0';
        output.resize(output.size() - kEscapeSequenceLen);
        return exitStatus;
    }

    exitStatus = output[output.size() - 1] - '0';
    output.resize(output.size() - 1);
    return exitStatus;
}

detail::Container::DockerAnswer
detail::Container::runCode(std::vector<std::string> && cmdLineArgs) {
    auto result = dockerWrapper.exec({.containerId = id, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    bool hasEscapeSequences = findEscapeSequences(result.message);
    int32_t const exitStatus = getExitCode(hasEscapeSequences, result.message);

    hasEscapeSequences = findEscapeSequences(result.message);
    if (hasEscapeSequences) {
        result.message.resize(result.message.size() - 2);
    }

    return {exitStatus, result.message};
}

detail::Container::DockerAnswer detail::Container::clean() {
    // TODO may be here we need to remove user code from container or in runCode
    return {.code = kSuccessCode};
}

detail::Container::Container(std::string id, ContainerType type)
    : id(std::move(id))
    , type(type) {}

bool detail::Container::DockerAnswer::isValid() const { return code == kSuccessCode; }

}  // namespace watchman
