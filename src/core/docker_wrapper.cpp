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

std::string DockerWrapper::run(DockerWrapper::RunParams && params) const { return {}; }

DockerWrapper::ExecResult DockerWrapper::exec(ExecParams && params) const { return {}; }

void DockerWrapper::put_archive(PutArchiveParams && params) const {}

}  // namespace watchman
