#pragma once
#include <docker.hpp>

#include <optional>
#include <string>
#include <vector>

namespace watchman {

class DockerWrapper {
public:
    DockerWrapper();
    explicit DockerWrapper(std::string const & host);

    struct RunParams {
        std::string const image;
        std::optional<bool> const tty;
        std::optional<size_t> const memoryLimit; // in bytes
    };
    std::string run(RunParams && params) const;

    struct ExecResult {
        int32_t exitCode{0};
        std::string output;
    };

    struct ExecParams {
        std::string const command;
        std::optional<bool> const detach;
        std::optional<bool> const tty;
    };
    ExecResult exec(ExecParams && params) const;

    std::vector<std::string> getAllContainers() const;
    bool isRunning(std::string const & id) const;
    std::string getImage(std::string const & id) const;
    void killContainer(std::string const & id) const;
    void removeContainer(std::string const & id) const;

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
