#include "container_manipulator.hpp"

#include "common/common.hpp"
#include "common/detail/containers.hpp"
#include "common/logging.hpp"

namespace watchman::detail {

ContainerOSManipulator::ContainerOSManipulator(Config && config,
                                               ProtectedContainers & protectedContainers)
    : m_protectedContainers(protectedContainers) {
    syncKillRunningContainers(config);
    syncLaunchNewContainers(config);
}

void ContainerOSManipulator::asyncRemoveContainerFromOs(std::string const & id) {
    unifex::schedule(m_containerKillerAliver.get_scheduler()) | unifex::then([this, &id] {
        m_dockerWrapper.killContainer(id);
        m_dockerWrapper.removeContainer(id);
    }) | unifex::sync_wait();
}

void ContainerOSManipulator::asyncCreateNewContainer(Config::ContainerType type,
                                                     std::string const & image) {
    unifex::schedule(m_containerKillerAliver.get_scheduler()) | unifex::then([this, type, image] {
        {
            std::scoped_lock lock(m_protectedContainers.mutex);
            RunContainer params;

            params.image = image;
            params.tty = true;
            params.memory = 300'000'000;

            std::string id = m_dockerWrapper.run(std::move(params));
            if (id.empty()) {
                Log::warning("Internal error: can't run container of type {}", type);
                return;
            }

            Log::info("Launch container: {}, id: {}", image, id);

            std::shared_ptr<BaseContainer> container;
            if (image.find("playground") != std::string::npos) {
                container = std::make_shared<PlaygroundContainer>(std::move(id), type);
            } else if (image.find("courses") != std::string::npos) {
                container = std::make_shared<CourseContainer>(std::move(id), type);
            } else {
                container = std::make_shared<PracticeContainer>(std::move(id), type);
            }

            m_protectedContainers.containers.at(type).push_back(container);
        }
        m_protectedContainers.containerFree.notify_all();
    }) | unifex::sync_wait();
}

void ContainerOSManipulator::syncKillRunningContainers(Config const & config) {
    auto const workingContainers = m_dockerWrapper.getAllContainers();

    // todo think about naming, containerTypes is awful
    auto const killContainers = [this, &workingContainers](auto && containerTypes) {
        for (auto const & [_, info] : containerTypes) {
            for (auto const & container : workingContainers) {
                if (container.image.find(info.imageName) != std::string::npos) {
                    if (m_dockerWrapper.isRunning(container.id)) {
                        m_dockerWrapper.killContainer(container.id);
                    }

                    Log::info("Remove container type: {}, id: {}", info.imageName, container.id);
                    m_dockerWrapper.removeContainer(container.id);
                }
            }
        }
    };

    killContainers(config.languages);
    killContainers(config.playgrounds);
    killContainers(config.practices);
}

void ContainerOSManipulator::syncLaunchNewContainers(Config const & config) {
    auto const launchContainers = [this](auto && containerTypes) {
        for (auto const & [type, info] : containerTypes) {
            std::vector<std::shared_ptr<BaseContainer>> containers;
            for (size_t index = 0; index < info.launched; ++index) {
                RunContainer params;

                params.image = info.imageName;
                params.tty = true;
                params.memory = 300'000'000;

                std::string id = m_dockerWrapper.run(std::move(params));
                if (id.empty()) {
                    Log::warning("Internal error: can't run container of type {}", type);
                    continue;
                }

                Log::info("Launch container: {}, id: {}", info.imageName, id);

                std::shared_ptr<BaseContainer> container;
                if (info.imageName.find("playground") != std::string::npos) {
                    container = std::make_shared<PlaygroundContainer>(std::move(id), type);
                } else if (info.imageName.find("course") != std::string::npos) {
                    container = std::make_shared<CourseContainer>(std::move(id), type);
                } else if (info.imageName.find("practice") != std::string::npos) {
                    container = std::make_shared<PracticeContainer>(std::move(id), type);
                } else {
                    throw std::logic_error{"Wrong image name: " + info.imageName};
                }

                containers.emplace_back(std::move(container));
            }

            if (!containers.empty()) {
                m_protectedContainers.containers.emplace(type, std::move(containers));
            }
        }
    };

    launchContainers(config.languages);
    launchContainers(config.playgrounds);
    launchContainers(config.practices);
}

}  // namespace watchman::detail
