#include "containers.hpp"

#include "common/detail/docker_end_answer.hpp"
#include "common/logging.hpp"
#include "core/detail/container_manipulator.hpp"

namespace watchman::detail {

static std::string const kUserSourceFile = "/home/code_runner";

BaseCodeLauncher::BaseCodeLauncher(std::string id, Config::CodeLauncherType type)
    : containerId(std::move(id))
    , type(std::move(type)) {}

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

LauncherRestartInfo BaseCodeLauncher::getRestartInfo() const {
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

CodeLauncherController::CodeLauncherController(Config && config)
    : m_manipulator(
          std::make_unique<ContainerOSManipulator>(std::move(config), m_protectedLaunchers)) {
    Log::info("Service launched");
}

std::unique_ptr<BaseCodeLauncher>
CodeLauncherController::getCodeLauncher(Config::CodeLauncherType const & type) {
    // lock for any container type
    // todo think of separating containers into different queues
    std::unique_lock lock(m_protectedLaunchers.mutex);

    auto & specificCodeLaunchers = m_protectedLaunchers.codeLaunchers.at(type);

    m_protectedLaunchers.containerFree.wait(
        lock, [&specificCodeLaunchers]() -> bool { return !specificCodeLaunchers.empty(); });

    auto launcher = std::move(specificCodeLaunchers.back());
    specificCodeLaunchers.pop_back();

    return launcher;
}

void CodeLauncherController::restartCodeLauncher(LauncherRestartInfo const & restartInfo) {
    std::string const id = restartInfo.containerId;
    std::string const image = restartInfo.image;
    {
        std::scoped_lock const lock(m_protectedLaunchers.mutex);

        auto & containers = m_protectedLaunchers.codeLaunchers.at(restartInfo.containerType);

        containers.erase(std::remove_if(containers.begin(), containers.end(),
                                        [id = restartInfo.containerId](auto const & c) {
                                            return c->containerId == id;
                                        }),
                         containers.end());
    }
    removeContainerFromOs(id);
    createNewContainer(restartInfo.containerType, image);
}

CodeLauncherController::~CodeLauncherController() = default;

PlaygroundCodeLauncher::PlaygroundCodeLauncher(std::string id, Config::CodeLauncherType type)
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

CourseCodeLauncher::CourseCodeLauncher(std::string id, Config::CodeLauncherType type)
    : BaseCodeLauncher(std::move(id), type) {}

RestartingLauncher::RestartingLauncher(std::unique_ptr<BaseCodeLauncher> container,
                                       std::function<void()> deleter)
    : codeLauncher(std::move(container))
    , m_releaser(std::move(deleter)) {}

RestartingLauncher::~RestartingLauncher() { m_releaser(); }

bool CodeLauncherController::launcherTypeIsValid(std::string const & name) const {
    return m_protectedLaunchers.codeLaunchers.contains(name);
}

void CodeLauncherController::removeContainerFromOs(std::string const & id) {
    m_manipulator->asyncRemoveContainerFromOs(id);
}

void CodeLauncherController::createNewContainer(Config::CodeLauncherType type,
                                                std::string const & image) {
    m_manipulator->asyncCreateNewContainer(type, image);
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

PracticeCodeLauncher::PracticeCodeLauncher(std::string id, Config::CodeLauncherType type)
    : BaseCodeLauncher(std::move(id), std::move(type)) {}

}  // namespace watchman::detail
