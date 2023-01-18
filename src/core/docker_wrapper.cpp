#include "docker_wrapper.hpp"

namespace watchman {

DockerWrapper::DockerWrapper() {}

DockerWrapper::DockerWrapper(std::string const & host)
    : m_docker(host) {}

std::vector<std::string> DockerWrapper::getAllContainers() const { return {}; }

bool DockerWrapper::isRunning(std::string const & id) const { return false; }

std::string DockerWrapper::getImage(std::string const & id) const { return {}; }

void DockerWrapper::killContainer(std::string const & id) const {}

void DockerWrapper::removeContainer(std::string const & id) const {}

std::string DockerWrapper::run(DockerRunParams && params) const { return {}; }

DockerExecResult DockerWrapper::exec(DockerExecParams && params) const { return {}; }

void DockerWrapper::putArchive(DockerPutArchiveParams && params) const {}

}  // namespace watchman
