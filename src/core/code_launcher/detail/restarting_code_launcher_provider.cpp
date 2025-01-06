#include "core/code_launcher/detail/restarting_code_launcher_provider.hpp"

#include "common/config.hpp"
#include "common/logging.hpp"
#include "core/code_launcher/detail/callbacked_code_launcher.hpp"
#include "core/code_launcher/detail/code_launchers.hpp"
#include "core/code_launcher/detail/container_manipulator.hpp"

namespace watchman::detail {

RestartingCodeLauncherProvider::RestartingCodeLauncherProvider(Config && config)
    : m_manipulator(std::make_unique<CodeLauncherOSManipulator>(std::move(config), m_storage)) {
    Log::info("Service launched");
}

std::unique_ptr<CodeLauncherInterface>
RestartingCodeLauncherProvider::getCodeLauncher(LanguageAction type) {
    if (!codeLauncherTypeIsValid(type)) {
        return nullptr;
    }

    auto launcher = m_storage.extract(type);

    auto ptr = std::make_unique<detail::CallbackedCodeLauncher>(
        std::move(launcher), [this, codeLauncherInfo = launcher->getInfo()]() {
            restartCodeLauncher(codeLauncherInfo);
        });

    return ptr;
}

void RestartingCodeLauncherProvider::restartCodeLauncher(CodeLauncherInfo const & restartInfo) {
    std::string const id = restartInfo.containerId;
    std::string const image = restartInfo.image;

    m_manipulator->asyncRemoveCodeLauncher(id);
    m_manipulator->asyncCreateCodeLauncher(restartInfo.type, image);
}

bool RestartingCodeLauncherProvider::codeLauncherTypeIsValid(LanguageAction const & name) const {
    return m_storage.contains(name);
}

RestartingCodeLauncherProvider::~RestartingCodeLauncherProvider() = default;

}  // namespace watchman::detail
