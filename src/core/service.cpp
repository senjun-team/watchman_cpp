#include "service.hpp"

#include "common/detail/docker_end_answer.hpp"
#include "common/logging.hpp"
#include "detail/container_manipulator.hpp"

namespace watchman {

Service::Service(Config && config)
    : m_containerController(std::move(config)) {}

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

std::vector<std::string> getPracticeDockerArgs(RunPracticeParams const & params) {
    std::string const cmd = "sh";
    std::string const script = "run.sh";
    std::vector<std::string> runArgs;

    runArgs.emplace_back(cmd);
    runArgs.emplace_back(script);

    // see run.sh for python pracitce
    auto runAction = [&runArgs, &params]() {
        runArgs.emplace_back(fmt::format("-p {}", params.pathToMainFile));
        runArgs.emplace_back(fmt::format("-o {}", params.userCmdLineArgs));
        runArgs.emplace_back(fmt::format("-r"));
    };

    auto testAction = [&runArgs, &params]() {
        runArgs.emplace_back(fmt::format("-f {}", params.practice.project.name));
        runArgs.emplace_back(fmt::format("-t"));
    };

    switch (params.practice.action) {
    case PracticeAction::Run: runAction(); break;
    case PracticeAction::Test: testAction(); break;
    }

    return runArgs;
}

Response Service::runTask(RunTaskParams const & runTaskParams) {
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

Response Service::runPlayground(RunProjectParams const & runProjectParams) {
    if (!m_containerController.containerNameIsValid(runProjectParams.containerType)) {
        Log::warning("Invalid container type: {}", runProjectParams.containerType);
        return {};
    }

    auto raiiContainer =
        getReadyContainer(runProjectParams.containerType);  // here we have got a race
    auto & container = raiiContainer.container;

    if (!container.prepareCode(makeProjectTar(std::move(runProjectParams.project)))) {
        Log::warning("Couldn't pass tar to container. Project name: {}",
                     runProjectParams.project.name);
        return {};
    }

    return container.runCode(getArgs(runProjectParams.project.name, runProjectParams.cmdLineArgs));
}

Response Service::runPractice(RunPracticeParams const & params) {
    if (!m_containerController.containerNameIsValid(params.containerType)) {
        Log::warning("Invalid container type: {}", params.containerType);
        return {};
    }

    auto raiiContainer = getReadyContainer(params.containerType);  // here we have got a race
    auto & container = raiiContainer.container;
    // here I want to process two cases
    // 1) just compile and run souce file

    if (!container.prepareCode(makeProjectTar(params.practice.project))) {
        Log::warning("Couldn't pass tar to container. Project name: {}",
                     params.practice.project.name);
        return {};
    }

    auto dockerCmdArgs = getPracticeDockerArgs(params);
    return container.runCode(std::move(dockerCmdArgs));
}

detail::ReleasingContainer Service::getReadyContainer(Config::ContainerType type) {
    auto & container = m_containerController.getReadyContainer(type);

    return {container,
            [this, &container]() { m_containerController.containerReleased(container); }};
}

}  // namespace watchman
