#include "service.hpp"

#include "common/detail/containers.hpp"
#include "common/logging.hpp"
#include "common/project.hpp"

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

    if (!m_containerController.launcherTypeIsValid(runTaskParams.containerType)) {
        Log::warning("Invalid container type: {}", runTaskParams.containerType);
        return {kInvalidCode,
                fmt::format("Invalid container type: {}", runTaskParams.containerType),
                std::string{}};
    }

    auto raiiCodeLaucher =
        getReadyLauncher(runTaskParams.containerType);  // here we have got a race
    auto & codeLauncher = raiiCodeLaucher.codeLauncher;

    std::vector<CodeFilename> data{{runTaskParams.sourceRun, kFilenameTask},
                                   {runTaskParams.sourceTest, kFilenameTaskTests}};

    auto result = codeLauncher->runCode(makeTar(std::move(data)),
                                        getArgs(kFilenameTask, runTaskParams.cmdLineArgs));
    if (errorCodeIsUnexpected(result.sourceCode)) {
        Log::error("Error return code {} from container {} type of {}", result.sourceCode,
                   codeLauncher->containerId, codeLauncher->type);
    }
    return result;
}

Response Service::runPlayground(RunProjectParams const & runProjectParams) {
    if (!m_containerController.launcherTypeIsValid(runProjectParams.containerType)) {
        Log::warning("Invalid container type: {}", runProjectParams.containerType);
        return {};
    }

    auto raiiLauncher =
        getReadyLauncher(runProjectParams.containerType);  // here we have got a race
    auto & codeLauncher = raiiLauncher.codeLauncher;
    return codeLauncher->runCode(
        makeProjectTar(std::move(runProjectParams.project)),
        getArgs(runProjectParams.project.name, runProjectParams.cmdLineArgs));
}

Response Service::runPractice(RunPracticeParams const & params) {
    if (!m_containerController.launcherTypeIsValid(params.containerType)) {
        Log::warning("Invalid container type: {}", params.containerType);
        return {};
    }

    auto raiiLauncher = getReadyLauncher(params.containerType);  // here we have got a race
    auto & codeLauncher = raiiLauncher.codeLauncher;
    auto dockerCmdArgs = getPracticeDockerArgs(params);

    return codeLauncher->runCode(makeProjectTar(params.practice.project), std::move(dockerCmdArgs));
}

detail::ReleasingContainer Service::getReadyLauncher(Config::CodeLauncherType type) {
    auto codeLauncher = m_containerController.getReadyCodeLauncher(type);
    detail::LauncherRestartInfo restartInfo{
        codeLauncher->containerId, codeLauncher->dockerWrapper.getImage(codeLauncher->containerId),
        codeLauncher->type};

    return {std::move(codeLauncher), [this, restartInfo = std::move(restartInfo)]() {
                m_containerController.restartCodeLauncher(restartInfo);
            }};
}

}  // namespace watchman
