#include "core/code_launcher/code_launcher_interface.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace watchman::detail {

class CallbackedCodeLauncher : public CodeLauncherInterface {
public:
    CallbackedCodeLauncher(std::unique_ptr<CodeLauncherInterface> launcher,
                           std::function<void()> deleter);
    ~CallbackedCodeLauncher();

    CallbackedCodeLauncher(CallbackedCodeLauncher const &) = delete;
    CallbackedCodeLauncher(CallbackedCodeLauncher &&) = delete;
    CallbackedCodeLauncher & operator=(CallbackedCodeLauncher const &) = delete;
    CallbackedCodeLauncher & operator=(CallbackedCodeLauncher &&) = delete;

    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;

    CodeLauncherInfo getInfo() const override;

private:
    std::unique_ptr<CodeLauncherInterface> m_codeLauncher;
    std::function<void()> m_releaser;
};

}  // namespace watchman::detail
