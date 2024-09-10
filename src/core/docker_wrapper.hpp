#pragma once
#include "common/common.hpp"

#include <memory>
#include <string>
#include <vector>

namespace watchman {

struct RunContainer {
    std::string image;
    bool tty{true};
    uint64_t memory{0};  // bytes
};

struct ExecResult {
    bool success{false};
    std::string message;
};

struct Exec {
    std::string containerId;
    std::vector<std::string> cmd;
};

struct Container {
    std::string id;
    std::string image;
};

struct PutArchive {
    std::string containerId;
    std::string path;
    std::string archive;  // in memory tar archive
    std::string copyUIDGID = "1";
};

class DockerWrapper {
public:
    DockerWrapper();
    ~DockerWrapper();

    std::string run(RunContainer && params);
    ExecResult exec(Exec && params);
    std::vector<Container> getAllContainers();
    bool isRunning(std::string const & id);
    std::string getImage(std::string const & id);
    bool killContainer(std::string const & id);
    bool removeContainer(std::string const & id);
    bool putArchive(PutArchive && params);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
}  // namespace watchman
