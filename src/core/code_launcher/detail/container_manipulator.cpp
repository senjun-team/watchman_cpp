#include "container_manipulator.hpp"

#include "common/config.hpp"
#include "common/logging.hpp"
#include "core/code_launcher/detail/code_launchers.hpp"

#include <unifex/sync_wait.hpp>
#include <unifex/then.hpp>

namespace watchman::detail {

std::unique_ptr<BaseCodeLauncher>
CodeLauncherOSManipulator::createCodeLauncher(CodeLauncherInfo const & info) {
    RunContainer params;

    params.image = info.image;
    params.tty = true;
    params.memory = 300'000'000;

    std::string id = m_dockerWrapper.run(std::move(params));
    if (id.empty()) {
        return nullptr;
    }

    Log::info("Launch container: {}, id: {}", info.image, id);

    std::unique_ptr<BaseCodeLauncher> container;
    if (info.image.find("playground") != std::string::npos) {
        container = std::make_unique<PlaygroundCodeLauncher>(std::move(id), info.type);
    } else if (info.image.find("course") != std::string::npos) {
        container = std::make_unique<CourseCodeLauncher>(std::move(id), info.type);
    } else if (info.image.find("practice") != std::string::npos) {
        container = std::make_unique<PracticeCodeLauncher>(std::move(id), info.type);
    } else {
        throw std::logic_error{"Wrong image name: " + info.image};
    }

    return container;
}

void CodeLauncherOSManipulator::removeCodeLauncher(std::string const & id) {
    if (m_dockerWrapper.isRunning(id)) {
        m_dockerWrapper.killContainer(id);
    }

    m_dockerWrapper.removeContainer(id);
}

CodeLauncherOSManipulator::CodeLauncherOSManipulator(Config && config, Storage & storage)
    : m_storage(storage) {
    syncRemoveRunningCodeLanchers(config);
    syncCreateCodeLaunchers(config);
}

void CodeLauncherOSManipulator::asyncRemoveCodeLauncher(std::string const & id) {
    unifex::schedule(m_containersContext.get_scheduler())
        | unifex::then([this, &id] { removeCodeLauncher(id); }) | unifex::sync_wait();
}

void CodeLauncherOSManipulator::asyncCreateCodeLauncher(TaskLauncherType type,
                                                        std::string const & image) {
    unifex::schedule(m_containersContext.get_scheduler()) | unifex::then([this, type, image] {
        auto container = createCodeLauncher({"", image, type});
        if (container == nullptr) {
            return;
        }
        m_storage.addValue(type, std::move(container));
    }) | unifex::sync_wait();
}

void CodeLauncherOSManipulator::syncRemoveRunningCodeLanchers(Config const & config) {
    auto const workingContainers = m_dockerWrapper.getAllContainers();

    // todo think about naming, containerTypes is awful
    auto const killContainers = [this, &workingContainers](auto && containerTypes) {
        for (auto const & [_, info] : containerTypes) {
            for (auto const & container : workingContainers) {
                if (container.image.find(info.imageName) != std::string::npos) {
                    Log::info("Remove container type: {}, id: {}", info.imageName, container.id);
                    removeCodeLauncher(container.id);
                }
            }
        }
    };

    killContainers(config.courses);
    killContainers(config.playgrounds);
    killContainers(config.practices);
}

void CodeLauncherOSManipulator::syncCreateCodeLaunchers(Config const & config) {
    auto const launchContainers = [this](auto && containerTypes) {
        for (auto const & [type, info] : containerTypes) {
            std::list<std::unique_ptr<BaseCodeLauncher>> containers;
            for (size_t index = 0; index < info.launched; ++index) {
                auto container = createCodeLauncher({"", info.imageName, type});
                if (container == nullptr) {
                    continue;
                }
                containers.emplace_back(std::move(container));
            }

            if (!containers.empty()) {
                m_storage.addValues(type, std::move(containers));
            }
        }
    };

    launchContainers(config.courses);
    launchContainers(config.playgrounds);
    launchContainers(config.practices);
}

}  // namespace watchman::detail
