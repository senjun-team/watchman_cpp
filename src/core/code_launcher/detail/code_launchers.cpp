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

std::map<Action, std::string_view> kDefaultPaths{{Action::Chapter, kTaskWorkdir},
                                                 {Action::Playground, kPlaygroundWorkdir},
                                                 {Action::Practice, kPracticeWorkdir}

};

BaseCodeLauncher::BaseCodeLauncher(std::string id, Language language)
    : containerId(std::move(id))
    , type(std::move(language)) {}

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

Response BaseCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                   std::vector<std::string> && cmdLineArgs, Action action) {
    if (!prepareCode(std::move(inMemoryTarWithSources), action)) {
        return {};
    }

    auto result = dockerWrapper.exec({.containerId = containerId, .cmd = std::move(cmdLineArgs)});

    if (!result.success) {
        return {result.success, result.message};
    }

    switch (action) {
    case Action::Chapter: return getCourseResponse(result.message);

    case Action::Playground: return getPlaygroungResponse(result.message);

    case Action::Practice: return getPracticeResponse(result.message);

    default: break;
    }

    throw std::logic_error{"Error: unknown action"};
}

CodeLauncherInfo BaseCodeLauncher::getInfo() const {
    return {containerId, dockerWrapper.getImage(containerId), type};
}

}  // namespace watchman
