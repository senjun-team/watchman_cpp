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
RestartingCodeLauncherProvider::getCodeLauncher(Language language) {
    if (!codeLauncherTypeIsValid(language)) {
        return nullptr;
    }

    auto launcher = m_storage.extract(language);
    auto info = launcher->getInfo();

    auto ptr = std::make_unique<detail::CallbackedCodeLauncher>(
        std::move(launcher),
        [this, restartInfo = std::move(info)]() { restartCodeLauncher(restartInfo); });

    return ptr;
}

void RestartingCodeLauncherProvider::restartCodeLauncher(CodeLauncherInfo const & restartInfo) {
    std::string const id = restartInfo.containerId;
    std::string const image = restartInfo.image;

    m_manipulator->asyncRemoveCodeLauncher(id);
    m_manipulator->asyncCreateCodeLauncher(restartInfo.type, image);
}

bool RestartingCodeLauncherProvider::codeLauncherTypeIsValid(Language language) const {
    return m_storage.contains(language);
}

RestartingCodeLauncherProvider::~RestartingCodeLauncherProvider() = default;

}  // namespace watchman::detail
