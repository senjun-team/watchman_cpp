#include "common/detail/containers.hpp"

#include "common/logging.hpp"
#include "core/code_launcher/code_launchers.hpp"
#include "core/detail/container_manipulator.hpp"

namespace watchman::detail {

CodeLauncherController::CodeLauncherController(Config && config)
    : m_manipulator(
          std::make_unique<CodeLauncherOSManipulator>(std::move(config), m_protectedLaunchers)) {
    Log::info("Service launched");
}

std::unique_ptr<CodeLauncherInterface>
CodeLauncherController::getCodeLauncher(Config::CodeLauncherType const & type) {
    // lock for any container type
    // todo think of separating containers into different queues
    std::unique_lock lock(m_protectedLaunchers.mutex);

    auto & specificCodeLaunchers = m_protectedLaunchers.codeLaunchers.at(type);

    m_protectedLaunchers.codeLauncherFree.wait(
        lock, [&specificCodeLaunchers]() -> bool { return !specificCodeLaunchers.empty(); });

    auto launcher = std::move(specificCodeLaunchers.back());
    specificCodeLaunchers.pop_back();

    return launcher;
}

void CodeLauncherController::restartCodeLauncher(CodeLauncherInfo const & restartInfo) {
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
    removeCodeLauncher(id);
    createNewCodeLauncher(restartInfo.containerType, image);
}

CodeLauncherController::~CodeLauncherController() = default;

RestartingCodeLauncher::RestartingCodeLauncher(std::unique_ptr<CodeLauncherInterface> container,
                                       std::function<void()> deleter)
    : m_codeLauncher(std::move(container))
    , m_releaser(std::move(deleter)) {}

RestartingCodeLauncher::~RestartingCodeLauncher() { m_releaser(); }

Response RestartingCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                     std::vector<std::string> && cmdLineArgs) {
    return m_codeLauncher->runCode(std::move(inMemoryTarWithSources), std::move(cmdLineArgs));
}

bool CodeLauncherController::codeLauncherTypeIsValid(std::string const & name) const {
    return m_protectedLaunchers.codeLaunchers.contains(name);
}

void CodeLauncherController::removeCodeLauncher(std::string const & id) {
    m_manipulator->asyncRemoveCodeLauncher(id);
}

void CodeLauncherController::createNewCodeLauncher(Config::CodeLauncherType type,
                                                std::string const & image) {
    m_manipulator->asyncCreateCodeLauncher(type, image);
}

}  // namespace watchman::detail
