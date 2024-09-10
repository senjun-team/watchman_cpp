#pragma once

#include "common/common.hpp"
#include "common/detail/containers.hpp"
#include "common/logging.hpp"
#include "core/docker_wrapper.hpp"

#include <unifex/single_thread_context.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/then.hpp>

namespace watchman::detail {

class ContainerOSManipulator {
public:
    ContainerOSManipulator(ProtectedContainers & protectedContainers)
        : m_protectedContainers(protectedContainers) {}
    void removeContainerFromOs(std::string const & id) {
        unifex::schedule(m_containerKillerAliver.get_scheduler()) | unifex::then([this, &id] {
            m_dockerWrapper.killContainer(id);
            m_dockerWrapper.removeContainer(id);
        }) | unifex::sync_wait();
    }

    void createNewContainer(Config::ContainerType type, std::string const & image) {
        unifex::schedule(m_containerKillerAliver.get_scheduler())
            | unifex::then([this, type, image] {
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
              })
            | unifex::sync_wait();
    }

private:
    ProtectedContainers & m_protectedContainers;

    unifex::single_thread_context m_containerKillerAliver;
    DockerWrapper m_dockerWrapper;
};

}  // namespace watchman::detail
