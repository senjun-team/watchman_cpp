#include "container_manipulator.hpp"

#include "common/api.hpp"
#include "common/config.hpp"
#include "common/logging.hpp"
#include "core/code_launcher/detail/code_launchers.hpp"

#include <stdexcept>
#include <unifex/sync_wait.hpp>
#include <unifex/then.hpp>

namespace watchman::detail {

// TODO remove
constexpr std::string_view kSenjunPattern = "senjun";

std::unique_ptr<BaseCodeLauncher>
CodeLauncherOSManipulator::createCodeLauncher(std::string const & image,
                                              LanguageAction languageAction) {
    RunContainer params;

    params.image = image;
    params.tty = true;
    params.memory =
        300'000'000'000;  // haskell needs more memory than others, TODO put it to the config

    std::string id = m_dockerWrapper.run(std::move(params));
    if (id.empty()) {
        return nullptr;
    }

    Log::info("Launch container: {}, id: {}", image, id);

    std::unique_ptr<BaseCodeLauncher> container;
    auto const l = languageAction.language;

    switch (languageAction.action) {
    case Action::Chapter: return std::make_unique<ChapterCodeLauncher>(std::move(id), l);
    case Action::Playground: return std::make_unique<PlaygroundCodeLauncher>(std::move(id), l);
    case Action::Practice: return std::make_unique<PracticeCodeLauncher>(std::move(id), l);
    }

    throw std::logic_error{"Unknorn image type while creating code launcher"};
}

void CodeLauncherOSManipulator::removeCodeLauncher(std::string const & id) {
    if (m_dockerWrapper.isRunning(id)) {
        m_dockerWrapper.killContainer(id);
    }

    m_dockerWrapper.removeContainer(id);
}

CodeLauncherOSManipulator::CodeLauncherOSManipulator(Config && config, Storage & storage)
    : m_storage(storage) {
    syncRemoveRunningCodeLanchers();
    syncCreateCodeLaunchers(config);
}

void CodeLauncherOSManipulator::asyncRemoveCodeLauncher(std::string const & id) {
    unifex::schedule(m_containersContext.get_scheduler())
        | unifex::then([this, &id] { removeCodeLauncher(id); }) | unifex::sync_wait();
}

void CodeLauncherOSManipulator::asyncCreateCodeLauncher(LanguageAction type,
                                                        std::string const & image) {
    unifex::schedule(m_containersContext.get_scheduler()) | unifex::then([this, type, image] {
        auto container = createCodeLauncher(image, type);
        if (container == nullptr) {
            return;
        }
        m_storage.addValue(type, std::move(container));
    }) | unifex::sync_wait();
}

void CodeLauncherOSManipulator::syncRemoveRunningCodeLanchers() {
    auto const workingContainers = m_dockerWrapper.getAllContainers();

    // todo think about naming, containerTypes is awful
    auto const killContainers = [this, &workingContainers](std::string_view pattern) {
        for (auto const & container : workingContainers) {
            if (container.image.find(pattern) != std::string::npos) {
                Log::info("Remove container type: {}, id: {}", container.image, container.id);
                removeCodeLauncher(container.id);
            }
        }
    };

    killContainers(kSenjunPattern);
}

void CodeLauncherOSManipulator::syncCreateCodeLaunchers(Config const & config) {
    auto const launchContainers = [this](auto && containerTypes, Action action) {
        for (auto const & [language, info] : containerTypes) {
            std::list<std::unique_ptr<BaseCodeLauncher>> containers;
            for (size_t index = 0; index < info.launched; ++index) {
                auto container = createCodeLauncher(info.imageName, {language, action});
                if (container == nullptr) {
                    continue;
                }
                containers.emplace_back(std::move(container));
            }

            if (!containers.empty()) {
                m_storage.addValues({language, action}, std::move(containers));
            }
        }
    };

    launchContainers(config.courses, Action::Chapter);
    launchContainers(config.playgrounds, Action::Playground);
    launchContainers(config.practices, Action::Practice);
}

}  // namespace watchman::detail
