#pragma once
#include "docker.hpp"

#include <optional>
#include <string>
#include <vector>

namespace watchman {

class DockerWrapper {
public:
    using ContainerId = std::string;
    using Image = std::string;

    DockerWrapper();
    explicit DockerWrapper(std::string const & host);

    struct RunParams {
        Image const image;
        std::optional<bool> const tty;
        std::optional<size_t> const memoryLimit;
    };

    ContainerId run(RunParams && params) const;
    std::vector<ContainerId> getAllContainers() const;
    bool isRunning(ContainerId const & id) const;
    Image getImage(ContainerId const & id) const;
    void killContainer(ContainerId const & id) const;
    void removeContainer(ContainerId const & id) const;

private:
    Docker m_docker;
};
}  // namespace watchman
