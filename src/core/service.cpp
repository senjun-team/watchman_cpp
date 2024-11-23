#include "service.hpp"

#include "common/logging.hpp"
#include "common/project.hpp"
#include "core/code_launcher/detail/code_launcher_controller.hpp"
#include "core/code_launcher/response.hpp"
#include "fmt/core.h"

namespace watchman {

Service::Service(Config && config)
    : m_codeLauncherController(
          std::make_unique<detail::CodeLauncherController>(std::move(config))) {}

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

    auto codeLauncher = m_codeLauncherController->getCodeLauncher(
        runTaskParams.containerType);  // here we have got a race

    if (codeLauncher == nullptr) {
        return {kInvalidCode,
                fmt::format("Probably wrong container type: {}", runTaskParams.containerType)};
    }

    std::vector<CodeFilename> data{{runTaskParams.sourceRun, kFilenameTask},
                                   {runTaskParams.sourceTest, kFilenameTaskTests}};

    auto result = codeLauncher->runCode(makeTar(std::move(data)),
                                        getArgs(kFilenameTask, runTaskParams.cmdLineArgs));
    if (errorCodeIsUnexpected(result.sourceCode)) {
        Log::error("Error return code {} from container type of {}", result.sourceCode,
                   runTaskParams.containerType);
    }
    return result;
}

Response Service::runPlayground(RunProjectParams const & runProjectParams) {
    auto codeLauncher = m_codeLauncherController->getCodeLauncher(
        runProjectParams.containerType);  // here we have got a race

    if (codeLauncher == nullptr) {
        return {};
    }

    return codeLauncher->runCode(
        makeProjectTar(std::move(runProjectParams.project)),
        getArgs(runProjectParams.project.name, runProjectParams.cmdLineArgs));
}

Response Service::runPractice(RunPracticeParams const & params) {
    auto codeLauncher =
        m_codeLauncherController->getCodeLauncher(params.containerType);  // here we have got a race

    if (codeLauncher == nullptr) {
        return {};
    }
    auto dockerCmdArgs = getPracticeDockerArgs(params);

    return codeLauncher->runCode(makeProjectTar(params.practice.project), std::move(dockerCmdArgs));
}

}  // namespace watchman
