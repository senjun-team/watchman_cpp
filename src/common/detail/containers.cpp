#include "common/detail/containers.hpp"

#include "common/logging.hpp"
#include "core/code_launcher/code_launchers.hpp"
#include "core/detail/container_manipulator.hpp"

namespace watchman::detail {

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

RestartingLauncher::RestartingLauncher(std::unique_ptr<BaseCodeLauncher> container,
                                       std::function<void()> deleter)
    : m_codeLauncher(std::move(container))
    , m_releaser(std::move(deleter)) {}

RestartingLauncher::~RestartingLauncher() { m_releaser(); }

Response RestartingLauncher::runCode(std::string && inMemoryTarWithSources,
                                     std::vector<std::string> && cmdLineArgs) {
    return m_codeLauncher->runCode(std::move(inMemoryTarWithSources), std::move(cmdLineArgs));
}

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

}  // namespace watchman::detail
