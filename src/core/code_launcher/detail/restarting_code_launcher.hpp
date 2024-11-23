#include "core/code_launcher/code_launcher_interface.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace watchman::detail {

class RestartingCodeLauncher : public CodeLauncherInterface {
public:
    RestartingCodeLauncher(std::unique_ptr<CodeLauncherInterface> launcher,
                           std::function<void()> deleter);
    ~RestartingCodeLauncher();

    RestartingCodeLauncher(RestartingCodeLauncher const &) = delete;
    RestartingCodeLauncher(RestartingCodeLauncher &&) = delete;
    RestartingCodeLauncher & operator=(RestartingCodeLauncher const &) = delete;
    RestartingCodeLauncher & operator=(RestartingCodeLauncher &&) = delete;

    Response runCode(std::string && inMemoryTarWithSources,
                     std::vector<std::string> && cmdLineArgs) override;

    CodeLauncherInfo getInfo() const override;

private:
    std::unique_ptr<CodeLauncherInterface> m_codeLauncher;
    std::function<void()> m_releaser;
};

}  // namespace watchman::detail
