#include "service.hpp"

#include <rapidjson/document.h>

namespace watchman {

Service::Service(std::string const & host)
    : m_containerController(host) {}

detail::ContainerController::ContainerController(std::string const & host)
    : m_docker(host) {
    readConfig();
}

void detail::ContainerController::readConfig() {
    // TODO fill hash maps from config
}

Response watchman::Service::runTask(watchman::RunTaskParams const & runTaskParams) {
    return {0, 0, "success"};
}

}  // namespace watchman
