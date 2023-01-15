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

void DockerWrapper::killContainer(DockerWrapper::ContainerId const & id) const {}

void DockerWrapper::removeContainer(DockerWrapper::ContainerId const & id) const {}

DockerWrapper::ContainerId DockerWrapper::run(DockerWrapper::RunParams && params) const {
    return {};
}

DockerWrapper::ExecResult DockerWrapper::exec(ExecParams && params) const { return {}; }

void DockerWrapper::put_archive(PutArchiveParams && params) const {}

}  // namespace watchman
