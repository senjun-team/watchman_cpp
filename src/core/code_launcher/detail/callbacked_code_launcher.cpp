#include "core/code_launcher/detail/callbacked_code_launcher.hpp"

namespace watchman::detail {

CallbackedCodeLauncher::CallbackedCodeLauncher(std::unique_ptr<CodeLauncherInterface> container,
                                               std::function<void()> deleter)
    : m_codeLauncher(std::move(container))
    , m_releaser(std::move(deleter)) {}

CallbackedCodeLauncher::~CallbackedCodeLauncher() { m_releaser(); }

Response CallbackedCodeLauncher::runCode(std::string && inMemoryTarWithSources,
                                         std::vector<std::string> && cmdLineArgs) {
    return m_codeLauncher->runCode(std::move(inMemoryTarWithSources), std::move(cmdLineArgs));
}

CodeLauncherInfo CallbackedCodeLauncher::getInfo() const { return m_codeLauncher->getInfo(); }

}  // namespace watchman::detail
