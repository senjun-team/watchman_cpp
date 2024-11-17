#include "container_manipulator.hpp"

#include "common/common.hpp"
#include "common/detail/containers.hpp"
#include "common/logging.hpp"

namespace watchman::detail {

std::unique_ptr<BaseContainer>
ContainerOSManipulator::createContainer(std::string const & type, std::string const & imageName) {
    RunContainer params;

    params.image = imageName;
    params.tty = true;
    params.memory = 300'000'000;

    std::string id = m_dockerWrapper.run(std::move(params));
    if (id.empty()) {
        Log::warning("Internal error: can't run container of type {}", type);
        return nullptr;
    }

    Log::info("Launch container: {}, id: {}", imageName, id);

    std::unique_ptr<BaseContainer> container;
    if (imageName.find("playground") != std::string::npos) {
        container = std::make_unique<PlaygroundContainer>(std::move(id), type);
    } else if (imageName.find("course") != std::string::npos) {
        container = std::make_unique<CourseContainer>(std::move(id), type);
    } else if (imageName.find("practice") != std::string::npos) {
        container = std::make_unique<PracticeContainer>(std::move(id), type);
    } else {
        throw std::logic_error{"Wrong image name: " + imageName};
    }

    return container;
}

void ContainerOSManipulator::killContainer(std::string const & id) {
    if (m_dockerWrapper.isRunning(id)) {
        m_dockerWrapper.killContainer(id);
    }

    m_dockerWrapper.removeContainer(id);
}

ContainerOSManipulator::ContainerOSManipulator(Config && config,
                                               ProtectedContainers & protectedContainers)
    : m_protectedContainers(protectedContainers) {
    syncKillRunningContainers(config);
    syncLaunchNewContainers(config);
}

void ContainerOSManipulator::asyncRemoveContainerFromOs(std::string const & id) {
    unifex::schedule(m_containersContext.get_scheduler())
        | unifex::then([this, &id] { killContainer(id); }) | unifex::sync_wait();
}

void ContainerOSManipulator::asyncCreateNewContainer(Config::ContainerType type,
                                                     std::string const & image) {
    unifex::schedule(m_containersContext.get_scheduler()) | unifex::then([this, type, image] {
        auto container = createContainer(type, image);
        {
            std::scoped_lock lock(m_protectedContainers.mutex);
            m_protectedContainers.containers.at(type).push_back(std::move(container));
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
                    Log::info("Remove container type: {}, id: {}", info.imageName, container.id);
                    killContainer(container.id);
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
            std::vector<std::unique_ptr<BaseContainer>> containers;
            for (size_t index = 0; index < info.launched; ++index) {
                auto container = createContainer(type, info.imageName);
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
