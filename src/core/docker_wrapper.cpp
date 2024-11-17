#include "docker_wrapper.hpp"

#include "common/common.hpp"
#include "common/logging.hpp"

#include <docker/answer.hpp>
#include <docker/client.hpp>
#include <docker/request_params.hpp>

namespace watchman {

class DockerWrapper::Impl {
public:
    std::string run(RunContainer && params);
    ExecResult exec(Exec && params);
    std::vector<Container> getAllContainers();
    bool isRunning(std::string const & id);
    std::string getImage(std::string const & id);
    bool killContainer(std::string const & id);
    bool removeContainer(std::string const & id);
    bool putArchive(PutArchive && params);

private:
    docker::Client m_docker;
};

std::vector<Container> DockerWrapper::Impl::getAllContainers() {
    docker::request_params::ListContainers params;
    params.all = true;
    auto response = m_docker.listContainers(params);
    if (!response.success) {
        Log::error("getAllContainers: answer error");
        return {};
    }

    if (response.containers.empty()) {
        return {};
    }

    std::vector<Container> containers;
    containers.reserve(response.containers.size());

    for (auto && responseContainer : response.containers) {
        Container container;
        container.id = std::move(responseContainer.id);
        container.image = std::move(responseContainer.image);
        containers.push_back(std::move(container));
    }

    return containers;
}

bool DockerWrapper::Impl::isRunning(std::string const & id) {
    auto const response = m_docker.inspectContainer(id);
    if (!response.success) {
        Log::error("isRunning: answer error for {}", id);
        return false;
    }

    return response.info.isRunning;
}

std::string DockerWrapper::Impl::getImage(std::string const & id) {
    auto const response = m_docker.inspectContainer(id);
    if (!response.success) {
        Log::error("getImage: answer error for {}", id);
        return {};
    }

    return response.info.image;
}

bool DockerWrapper::Impl::killContainer(std::string const & id) {
    if (id.empty()) {
        Log::error("killContainer error: id is empty");
        return false;
    }

    auto const responseKill = m_docker.killContainer({id, {}});
    if (!responseKill.success) {
        Log::error("killContainer parse error for {}", id);
        return false;
    }

    return true;
}

bool DockerWrapper::Impl::removeContainer(std::string const & id) {
    if (id.empty()) {
        Log::error("removeContainer error: id is empty");
        return false;
    }

    auto const responseKill = m_docker.deleteContainer({id, {}, {}});
    if (!responseKill.success) {
        Log::error("removeContainer parse error for {}", id);
        return false;
    }

    return true;
}

std::string DockerWrapper::Impl::run(RunContainer && params) {
    docker::request_params::RunContainer dockerParams;
    dockerParams.image = std::move(params.image);
    dockerParams.tty = params.tty;
    dockerParams.memory = params.memory;

    auto response = m_docker.runContainer(std::move(dockerParams));
    if (!response.success) {
        Log::error("run container error for {} image", dockerParams.image);
        return {};
    }

    return response.containerId;
}

bool DockerWrapper::Impl::putArchive(PutArchive && params) {
    docker::request_params::PutArchive dockerParams;
    dockerParams.containerId = std::move(params.containerId);
    dockerParams.path = std::move(params.path);
    dockerParams.archive = std::move(params.archive);
    dockerParams.copyUIDGID = std::move(params.copyUIDGID);

    auto result = m_docker.putArchive(dockerParams);
    if (!result.success) {
        Log::error("putArchive parse error: {}", result.message);
        return false;
    }

    return true;
}

ExecResult DockerWrapper::Impl::exec(Exec && params) {
    docker::request_params::ExecCreate dockerParams;
    dockerParams.containerId = std::move(params.containerId);
    dockerParams.cmd = std::move(params.cmd);

    LogDuration duration("Exec time");
    auto result = m_docker.exec(std::move(dockerParams));
    return {result.success, result.message};
}

DockerWrapper::DockerWrapper()
    : m_impl(std::make_unique<DockerWrapper::Impl>()) {}

DockerWrapper::~DockerWrapper() {}

std::string DockerWrapper::run(RunContainer && params) { return m_impl->run(std::move(params)); }

ExecResult DockerWrapper::exec(Exec && params) { return m_impl->exec(std::move(params)); }

std::vector<Container> DockerWrapper::getAllContainers() { return m_impl->getAllContainers(); }

bool DockerWrapper::isRunning(std::string const & id) { return m_impl->isRunning(id); }

std::string DockerWrapper::getImage(std::string const & id) { return m_impl->getImage(id); }

bool DockerWrapper::killContainer(std::string const & id) { return m_impl->killContainer(id); }

bool DockerWrapper::removeContainer(std::string const & id) { return m_impl->removeContainer(id); }

bool DockerWrapper::putArchive(PutArchive && params) {
    return m_impl->putArchive(std::move(params));
}

}  // namespace watchman
