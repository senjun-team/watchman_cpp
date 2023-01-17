#pragma once
#include <docker.hpp>
#include <optional>
#include <string>
#include <vector>

namespace watchman {

struct DockerRunParams {
    std::string const image;
    std::optional<bool> const tty;
    std::optional<size_t> const memoryLimit;  // in bytes
};

struct DockerExecResult {
    int32_t exitCode{0};
    std::string output;
};

struct DockerExecParams {
    std::string const command;
    std::optional<bool> const detach;
    std::optional<bool> const tty;
};

struct DockerPutArchiveParams {
    std::string const containerId;
    std::string const pathInContainer;
    std::string const pathToArchive;
};

class DockerWrapper {
public:
    DockerWrapper();
    explicit DockerWrapper(std::string const & host);

    DockerWrapper(DockerWrapper const &) = delete;
    DockerWrapper & operator=(DockerWrapper const &) = delete;
    DockerWrapper const & operator=(DockerWrapper const &) const = delete;

    std::string run(DockerRunParams && params) const;
    DockerExecResult exec(DockerExecParams && params) const;
    std::vector<std::string> getAllContainers() const;
    bool isRunning(std::string const & id) const;
    std::string getImage(std::string const & id) const;
    void killContainer(std::string const & id) const;
    void removeContainer(std::string const & id) const;
    void putArchive(DockerPutArchiveParams && params) const;

private:
    Docker m_docker;
};
}  // namespace watchman
