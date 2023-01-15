#include "docker_wrapper.hpp"

namespace watchman {

DockerWrapper::DockerWrapper() {}

DockerWrapper::DockerWrapper(std::string const & host)
    : m_docker(host) {}

std::vector<DockerWrapper::ContainerId> DockerWrapper::getAllContainers() const { return {}; }

bool DockerWrapper::isRunning(DockerWrapper::ContainerId const & id) const { return false; }

DockerWrapper::Image DockerWrapper::getImage(DockerWrapper::ContainerId const & id) const {
    return {};
}

void DockerWrapper::killContainer(const DockerWrapper::ContainerId & id) const {}

void DockerWrapper::removeContainer(const DockerWrapper::ContainerId & id) const {}

DockerWrapper::ContainerId DockerWrapper::run(DockerWrapper::RunParams && params) const {
    return {};
}

}  // namespace watchman
