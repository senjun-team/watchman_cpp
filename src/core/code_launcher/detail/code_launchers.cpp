#include "core/code_launcher/detail/code_launchers.hpp"

#include "common/logging.hpp"

// crutch, redo!
#include "core/code_launcher/detail/docker_end_answer.hpp"

namespace watchman {

static std::string const kUserSourceFile = "/home/code_runner";

bool BaseCodeLauncher::prepareCode(std::string && tarString) {
    PutArchive params;
    params.containerId = containerId;
    params.path = kUserSourceFile;
    params.archive = std::move(tarString);
    params.copyUIDGID = "1";

    if (!dockerWrapper.putArchive(std::move(params))) {
        Log::error("Couldn't put an archive to container");
        return false;
    }

    return true;
}

Response PlaygroundCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                         std::vector<std::string> && cmdLineArgs) {
    if (!prepareCode(std::move(inMemoryTarWithSources))) {
        return {};
    }

    auto result = dockerWrapper.exec({.containerId = containerId, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getPlaygroungResponse(result.message);
}

PracticeCodeLauncher::PracticeCodeLauncher(std::string id, TaskLauncherType type)
    : BaseCodeLauncher(std::move(id), std::move(type)) {}

PlaygroundCodeLauncher::PlaygroundCodeLauncher(std::string id, TaskLauncherType type)
    : BaseCodeLauncher(std::move(id), type) {}

Response CourseCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                     std::vector<std::string> && cmdLineArgs) {
    if (!prepareCode(std::move(inMemoryTarWithSources))) {
        return {};
    }
    auto result = dockerWrapper.exec({.containerId = containerId, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getCourseResponse(result.message);
}

CourseCodeLauncher::CourseCodeLauncher(std::string id, TaskLauncherType type)
    : BaseCodeLauncher(std::move(id), type) {}

BaseCodeLauncher::BaseCodeLauncher(std::string id, TaskLauncherType type)
    : containerId(std::move(id))
    , type(std::move(type)) {}

CodeLauncherInfo BaseCodeLauncher::getInfo() const {
    return {containerId, dockerWrapper.getImage(containerId), type};
}

Response PracticeCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                       std::vector<std::string> && dockerCmdLineArgs) {
    if (!prepareCode(std::move(inMemoryTarWithSources))) {
        return {};
    }

    auto result =
        dockerWrapper.exec({.containerId = containerId, .cmd = std::move(dockerCmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getPracticeResponse(result.message);
}

}  // namespace watchman
