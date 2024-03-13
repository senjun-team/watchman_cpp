#include "service.hpp"

#include "common/logging.hpp"
#include "core/docker_wrapper.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <cctype>
#include <map>
#include <string_view>

namespace watchman {

enum class ExitCodes : int32_t {
    Ok,         // no errors
    UserError,  // error: syntax / building / bad running user code
    TestError,  // test case failed
    Unknown     // trash answer from container
};

// 3 bytes: code\r\n
std::string_view constexpr kEscapeSequence = "\r\n";
std::string_view constexpr kCodeTestsSeparator = "user_code_ok_f936a25e";

std::string_view constexpr kWrongDockerImage = "Maybe wrong docker image?";

struct MarkToStatusCode {
    std::string_view string;
    ExitCodes code;
};

std::vector<MarkToStatusCode> const kCorrelations{
    {"user_solution_ok_f936a25e", ExitCodes::Ok},
    {"user_solution_error_f936a25e", ExitCodes::UserError},
    {"tests_cases_error_f936a25e", ExitCodes::TestError}};

size_t getStringLength(ExitCodes code) {
    switch (code) {
    case ExitCodes::Ok: return kCorrelations[0].string.size();
    case ExitCodes::UserError: return kCorrelations[1].string.size();
    case ExitCodes::TestError: return kCorrelations[2].string.size();
    case ExitCodes::Unknown: return -1;
    }
}

// debug function for macos purposes
void removeEscapeSequences(std::string & string) {
    size_t constexpr size = kEscapeSequence.size();
    while (size_t const from = string.find(kEscapeSequence)) {
        if (from == std::string::npos) {
            break;
        }
        string.erase(from, size);
    }
}

struct ContainerMessages {
    std::string usersOutput;
    std::string testsOutput;
};

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

detail::Container & detail::ContainerController::getReadyContainer(Config::ContainerType type) {
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

void detail::ContainerController::containerReleased(Container & container) {
    {
        std::scoped_lock const lock(m_mutex);
        container.isReserved = false;
    }
    m_containerFree.notify_one();
}

detail::ContainerController::~ContainerController() = default;

void detail::ContainerController::killOldContainers(
    DockerWrapper & dockerWrapper,
    std::unordered_map<Config::ContainerType, Language> const & languages) {
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
    std::unordered_map<Config::ContainerType, Language> const & languages) {
    for (auto const & [type, language] : languages) {
        std::vector<std::shared_ptr<Container>> containers;
        for (size_t index = 0; index < language.launched; ++index) {
            RunContainer params;

            params.image = language.imageName;
            params.tty = true;
            params.memory = 300'000'000;

            std::string id = dockerWrapper.run(std::move(params));
            if (id.empty()) {
                Log::warning("Internal error: can't run container of type {}", type);
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
    if (!m_containerController.containerNameIsValid(runTaskParams.containerType)) {
        Log::warning("Invalid container type is provided");
        return {.sourceCode = kInvalidCode,
                .output = fmt::format("Error: Invalid container type \'{}\'",
                                      runTaskParams.containerType),
                .testsOutput = ""};
    }

    auto raiiContainer = getReadyContainer(runTaskParams.containerType);  // here we have got a race
    auto & container = raiiContainer.container;
    if (!container.prepareCode(runTaskParams.sourceRun, runTaskParams.sourceTest)) {
        Log::warning("Couldn't pass tar to container");
        return {.sourceCode = kInvalidCode,
                .output = fmt::format("Couldn't pass tar to container"),
                .testsOutput = ""};
    }

    Response resultSourceRun;

    if (!runTaskParams.sourceRun.empty()) {
        resultSourceRun = container.runCode(getArgs(kFilenameTask, runTaskParams.cmdLineArgs));
        return resultSourceRun;
    }

    return {kUserCodeError, "Empty user code", ""};
}

detail::ReleasingContainer Service::getReadyContainer(Config::ContainerType type) {
    auto & container = m_containerController.getReadyContainer(type);

    return {container,
            [this, &container]() { m_containerController.containerReleased(container); }};
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

bool hasEscapeSequences(std::string const & output) {
    if (output.size() < kEscapeSequence.size()) {
        return false;
    }

    return output.ends_with(kEscapeSequence);
}

ContainerMessages getOutputsWithEscapeSequence(ExitCodes code, std::string const & messages) {
    size_t const userCodeEndIndex = messages.find(kCodeTestsSeparator);
    std::string usersOutput = userCodeEndIndex > kEscapeSequence.size()
                                ? messages.substr(0, userCodeEndIndex - kEscapeSequence.size())
                                : std::string{};

    size_t const from = userCodeEndIndex + kCodeTestsSeparator.size() + kEscapeSequence.size();
    size_t const amount = messages.size() - from - getStringLength(code) - kEscapeSequence.size();
    std::string testsOutput = messages.substr(from, amount);
    if (testsOutput.ends_with(kEscapeSequence)) {
        testsOutput = testsOutput.substr(0, testsOutput.size() - kEscapeSequence.size());
    }

    return {std::move(usersOutput), std::move(testsOutput)};
}

ContainerMessages getOutputsNormal(ExitCodes code, std::string const & messages) {
    size_t const userCodeEndIndex = messages.find(kCodeTestsSeparator);
    std::string usersOutput = userCodeEndIndex == 0 ? std::string{}
                                                    : messages.substr(0, userCodeEndIndex);

    size_t const from = userCodeEndIndex + kCodeTestsSeparator.size();
    size_t const amount = messages.size() - from - getStringLength(code);
    std::string testsOutput = messages.substr(from, amount);

    return {std::move(usersOutput), std::move(testsOutput)};
}

ExitCodes getSequencedExitCode(std::string const & containerOutput) {
    // Case when timeout util returns its status code
    for (auto const & [codeStr, code] : kTimeoutUtilCodes) {
        if (containerOutput.ends_with(codeStr)) {
            return static_cast<ExitCodes>(code);
        }
    }

    for (auto const & pattern : kCorrelations) {
        if (containerOutput.find(pattern.string) != std::string::npos) {
            return pattern.code;
        }
    }

    return ExitCodes::Unknown;
}

std::string getUserOutput(std::string const & message) {
    return message.substr(0, message.size() - kEscapeSequence.size()
                                 - getStringLength(ExitCodes::UserError));
}

std::string getUserOutputNormal(std::string const & message) {
    return message.substr(0, message.size() - getStringLength(ExitCodes::UserError));
}

Response processSequenceEndedMessage(std::string const & message) {
    ExitCodes const exitCode = getSequencedExitCode(message);
    switch (exitCode) {
    case ExitCodes::Ok: {
        // pattern: user's_output user_code_ok_f936a25e user_solution_ok_f936a25e
        auto [usersOutput, testsOutput] = getOutputsWithEscapeSequence(ExitCodes::Ok, message);
        return {kSuccessCode, std::move(usersOutput), std::move(testsOutput)};
    }

    case ExitCodes::UserError: {
        // pattern: user's_output user_solution_error_f936a25e
        return {kUserCodeError, getUserOutput(message), std::string{}};
    }

    case ExitCodes::TestError: {
        // pattern: user's_output user_code_ok_f936a25e tests_cases_error_f936a25e
        auto [usersOutput, testsOutput] =
            getOutputsWithEscapeSequence(ExitCodes::TestError, message);
        return {kTestsError, std::move(usersOutput), std::move(testsOutput)};
    }

    default:
        Log::warning(kWrongDockerImage);
        return {static_cast<int32_t>(exitCode), "Internal error", ""};
    }
}

Response processNormalEndedMessage(std::string const & message) {
    ExitCodes const exitCode = getSequencedExitCode(message);
    switch (exitCode) {
    case ExitCodes::Ok: {
        // pattern: user's_output user_code_ok_f936a25e user_solution_ok_f936a25e
        auto [usersOutput, testsOutput] = getOutputsNormal(ExitCodes::Ok, message);
        return {kSuccessCode, std::move(usersOutput), std::move(testsOutput)};
    }

    case ExitCodes::UserError: {
        // pattern: user's_output user_solution_error_f936a25e
        return {kUserCodeError, getUserOutputNormal(message), std::string{}};
    }

    case ExitCodes::TestError: {
        // pattern: user's_output user_code_ok_f936a25e tests_cases_error_f936a25e
        auto [usersOutput, testsOutput] = getOutputsNormal(ExitCodes::TestError, message);
        return {kTestsError, std::move(usersOutput), std::move(testsOutput)};
    }

    default:
        Log::warning(kWrongDockerImage);
        return {static_cast<int32_t>(exitCode), "Internal error", ""};
    }
}

Response detail::Container::runCode(std::vector<std::string> && cmdLineArgs) {
    auto result = dockerWrapper.exec({.containerId = id, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    if (hasEscapeSequences(result.message)) {
        return processSequenceEndedMessage(result.message);
    }

    return processNormalEndedMessage(result.message);
}

detail::Container::Container(std::string id, Config::ContainerType type)
    : id(std::move(id))
    , type(std::move(type)) {}

detail::ReleasingContainer::ReleasingContainer(detail::Container & container,
                                               std::function<void()> deleter)
    : container(container)
    , m_releaser(std::move(deleter)) {}

detail::ReleasingContainer::~ReleasingContainer() { m_releaser(); }

bool detail::ContainerController::containerNameIsValid(const std::string & name) const {
    return m_containers.contains(name);
}
}  // namespace watchman
