#include "docker_wrapper.hpp"

#include "common/logging.hpp"

#include <docker/request_params.hpp>

namespace watchman {

// there's no string_view because of operator[] inside rapidjson::Value
static std::string const kId = "Id";
static std::string const kData = "data";
static std::string const kSuccess = "success";
static std::string const kImage = "Image";
static std::string const kState = "State";
static std::string const kRunning = "Running";
static std::string const kConfig = "Config";

std::vector<docker::Container> DockerWrapper::getAllContainers() {
    docker::request_params::ListContainers params;
    params.all = true;
    auto response = m_docker.listContainers(params);
    if (!response.success) {
        Log::error("getAllContainers: answer error");
        return {};
    }

    return response.containers;
}

bool DockerWrapper::isRunning(std::string const & id) {
    auto const response = m_docker.inspectContainer(id);
    if (!response.success) {
        Log::error("isRunning: answer error for {}", id);
        return false;
    }

    return response.isRunning;
}

std::string DockerWrapper::getImage(std::string const & id) {
    auto const response = m_docker.inspectContainer(id);
    if (!response.success) {
        Log::error("getImage: answer error for {}", id);
        return {};
    }

    return response.image;
}

bool DockerWrapper::killContainer(std::string const & id) {
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

bool DockerWrapper::removeContainer(std::string const & id) {
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

std::string DockerWrapper::run(docker::request_params::RunContainer && params) {
    auto response = m_docker.runContainer(std::move(params));
    if (!response.success) {
        Log::error("run container error for {} image", params.image);
        return {};
    }

    return response.containerId;
}

bool DockerWrapper::putArchive(docker::request_params::PutArchive && params) {
    auto result = m_docker.putArchive(params);
    if (!result.success) {
        Log::error("putArchive parse error: {}", result.message);
        return false;
    }

    return true;
}

docker::answer::Exec DockerWrapper::exec(docker::request_params::ExecCreate && params) {
    LogDuration duration("Exec time");
    return m_docker.exec(std::move(params));
}

}  // namespace watchman
