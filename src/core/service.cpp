#include "service.hpp"

#include "common/common.hpp"
#include "common/logging.hpp"
#include "common/project.hpp"
#include "core/code_launcher/detail/restarting_code_launcher_provider.hpp"
#include "core/code_launcher/response.hpp"
#include "core/detail/data_run_prepare.hpp"

namespace watchman {

Service::Service(Config && config)
    : m_codeLauncherProvider(
          std::make_unique<detail::RestartingCodeLauncherProvider>(std::move(config))) {}

std::vector<std::string> getArgs(std::string type, std::string const & filename,
                                 std::vector<std::string> const & cmdLineArgs) {
    std::string const cmd = "sh";

    std::vector<std::string> runArgs;

    runArgs.reserve(3 + cmdLineArgs.size());

    runArgs.emplace_back(cmd);
    runArgs.emplace_back(std::move(type));
    runArgs.emplace_back(fmt::format("-f {}", filename));

    runArgs.insert(runArgs.end(), cmdLineArgs.begin(), cmdLineArgs.end());
    return runArgs;
}

// Returns vector containing sequence: cmd, script, filename, args
std::vector<std::string> getArgsCourse(std::string const & filename,
                                       std::vector<std::string> const & cmdLineArgs) {
    constexpr std::string kTaskScript = "run-task.sh";
    return getArgs(kTaskScript, filename, cmdLineArgs);
}

std::vector<std::string> getArgsPlayground(std::string const & filename,
                                           std::vector<std::string> const & cmdLineArgs) {
    constexpr std::string kTaskScript = "run-playground.sh";
    return getArgs(kTaskScript, filename, cmdLineArgs);
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

Response Service::runTask(CourseTaskParams const & runTaskParams) {
    if (runTaskParams.sourceRun.empty() && runTaskParams.sourceTest.empty()) {
        Log::warning("Empty files with code and test");
        return {kUserCodeError, "Sources and tests are not provided", ""};
    }

    auto codeLauncher = m_codeLauncherProvider->getCodeLauncher(
        runTaskParams.taskLauncherType);  // here we have got a race

    if (codeLauncher == nullptr) {
        return {kInvalidCode, fmt::format("Probably wrong containerType")};
    }

    auto result = codeLauncher->runCode(detail::prepareData(runTaskParams),
                                        getArgsCourse(kFilenameTask, runTaskParams.cmdLineArgs));
    if (errorCodeIsUnexpected(result.sourceCode)) {
        Log::error("Error return code {} from image {}", result.sourceCode,
                   codeLauncher->getInfo().image);
    }
    return result;
}

Response Service::runPlayground(PlaygroundTaskParams const & runProjectParams) {
    auto codeLauncher = m_codeLauncherProvider->getCodeLauncher(
        runProjectParams.taskLauncherType);  // here we have got a race

    if (codeLauncher == nullptr) {
        return {};
    }

    return codeLauncher->runCode(
        detail::prepareData(runProjectParams),
        getArgsPlayground(runProjectParams.project.name, runProjectParams.cmdLineArgs));
}

Response Service::runPractice(RunPracticeParams const & params) {
    auto codeLauncher = m_codeLauncherProvider->getCodeLauncher(
        params.taskLauncherType);  // here we have got a race

    if (codeLauncher == nullptr) {
        return {};
    }

    auto dockerCmdArgs = getPracticeDockerArgs(params);
    return codeLauncher->runCode(detail::prepareData(params), std::move(dockerCmdArgs));
}

}  // namespace watchman
