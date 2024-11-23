#include "restarting_code_launcher.hpp"

namespace watchman::detail {

RestartingCodeLauncher::RestartingCodeLauncher(std::unique_ptr<CodeLauncherInterface> container,
                                               std::function<void()> deleter)
    : m_codeLauncher(std::move(container))
    , m_releaser(std::move(deleter)) {}

RestartingCodeLauncher::~RestartingCodeLauncher() { m_releaser(); }

Response RestartingCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                         std::vector<std::string> && cmdLineArgs) {
    return m_codeLauncher->runCode(std::move(inMemoryTarWithSources), std::move(cmdLineArgs));
}

CodeLauncherInfo RestartingCodeLauncher::getInfo() const { return m_codeLauncher->getInfo(); }

}  // namespace watchman::detail
