#pragma once
#include "common/common.hpp"

#include <docker/answer.hpp>
#include <docker/client.hpp>
#include <docker/request_params.hpp>
#include <optional>
#include <string>
#include <vector>

namespace watchman {

class DockerWrapper {
public:
    std::string run(docker::request_params::RunContainer && params);
    docker::answer::Exec exec(docker::request_params::ExecCreate && params);
    std::vector<docker::Container> getAllContainers();
    bool isRunning(std::string const & id);
    std::string getImage(std::string const & id);
    bool killContainer(std::string const & id);
    bool removeContainer(std::string const & id);
    bool putArchive(docker::request_params::PutArchive && params);

private:
    docker::Client m_docker;
};
}  // namespace watchman
