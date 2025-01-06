#pragma once

#include "common/config.hpp"
#include "core/code_launcher/code_launcher_provider_interface.hpp"
#include "core/code_launcher/detail/container_manipulator.hpp"
#include "core/code_launcher/detail/storage.hpp"

namespace watchman::detail {

// Restarts code launcher after usage
class RestartingCodeLauncherProvider : public CodeLauncherProviderInterface {
public:
    explicit RestartingCodeLauncherProvider(Config && config);
    ~RestartingCodeLauncherProvider();

    RestartingCodeLauncherProvider(RestartingCodeLauncherProvider const & other) = delete;
    RestartingCodeLauncherProvider(RestartingCodeLauncherProvider && other) = delete;
    RestartingCodeLauncherProvider &
    operator=(RestartingCodeLauncherProvider const & other) = delete;
    RestartingCodeLauncherProvider & operator=(RestartingCodeLauncherProvider && other) = delete;

    std::unique_ptr<CodeLauncherInterface> getCodeLauncher(LanguageAction type) override;

private:
    void restartCodeLauncher(CodeLauncherInfo const & info);
    bool codeLauncherTypeIsValid(LanguageAction const & name) const;

    ExtractingStorage<LanguageAction, BaseCodeLauncher, LanguageActionHasher> m_storage;
    std::unique_ptr<CodeLauncherOSManipulator> m_manipulator;
};

}  // namespace watchman::detail
