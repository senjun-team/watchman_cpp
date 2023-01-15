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

    struct ExecResult {
        int32_t exitCode{0};
        std::string output;
    };

    struct ExecParams {
        std::string const command;
        std::optional<bool> detach;
        std::optional<bool> tty;
    };
    ExecResult exec(ExecParams && params) const;

    std::vector<ContainerId> getAllContainers() const;
    bool isRunning(ContainerId const & id) const;
    Image getImage(ContainerId const & id) const;
    void killContainer(ContainerId const & id) const;
    void removeContainer(ContainerId const & id) const;

    struct PutArchiveParams {
        std::string const containerId;
        std::string const pathInContainer;
        std::string const pathToArchive;
    };
    void put_archive(PutArchiveParams && params) const;

private:
    Docker m_docker;
};
}  // namespace watchman
