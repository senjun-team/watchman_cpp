#include "core/code_launcher/detail/code_launchers.hpp"

#include "common/config.hpp"
#include "common/logging.hpp"

#include <map>
#include <stdexcept>

// crutch, redo!
#include "core/code_launcher/detail/docker_end_answer.hpp"

namespace watchman {

constexpr std::string_view kTaskWorkdir = "/home/code_runner/task";
constexpr std::string_view kPlaygroundWorkdir = "/home/code_runner/playground";
constexpr std::string_view kPracticeWorkdir = "/home/code_runner/practice";

std::map<Action, std::string_view> kDefaultPaths{{Action::ChapterTask, kTaskWorkdir},
                                                 {Action::Playground, kPlaygroundWorkdir},
                                                 {Action::Practice, kPracticeWorkdir}

};

bool BaseCodeLauncher::prepareCode(std::string && tarString, Action type) {
    auto defaultPath = kDefaultPaths.find(type);
    if (defaultPath == kDefaultPaths.end()) {
        throw std::logic_error{"BaseCodeLauncher::prepareCode: unknown type"};
    }

    PutArchive params;
    params.containerId = containerId;
    params.path = defaultPath->second;
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
    if (!prepareCode(std::move(inMemoryTarWithSources), Action::Playground)) {
        return {};
    }

    auto result = dockerWrapper.exec({.containerId = containerId, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getPlaygroungResponse(result.message);
}

PracticeCodeLauncher::PracticeCodeLauncher(std::string id, LanguageAction type)
    : BaseCodeLauncher(std::move(id), std::move(type)) {}

PlaygroundCodeLauncher::PlaygroundCodeLauncher(std::string id, LanguageAction type)
    : BaseCodeLauncher(std::move(id), type) {}

Response CourseCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                     std::vector<std::string> && cmdLineArgs) {
    if (!prepareCode(std::move(inMemoryTarWithSources), Action::ChapterTask)) {
        return {};
    }
    auto result = dockerWrapper.exec({.containerId = containerId, .cmd = std::move(cmdLineArgs)});
    if (!result.success) {
        return {result.success, result.message};
    }

    return getCourseResponse(result.message);
}

CourseCodeLauncher::CourseCodeLauncher(std::string id, LanguageAction type)
    : BaseCodeLauncher(std::move(id), type) {}

BaseCodeLauncher::BaseCodeLauncher(std::string id, LanguageAction type)
    : containerId(std::move(id))
    , type(std::move(type)) {}

CodeLauncherInfo BaseCodeLauncher::getInfo() const {
    return {containerId, dockerWrapper.getImage(containerId), type};
}

Response PracticeCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                       std::vector<std::string> && dockerCmdLineArgs) {
    if (!prepareCode(std::move(inMemoryTarWithSources), Action::Practice)) {
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
