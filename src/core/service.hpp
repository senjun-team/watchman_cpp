#pragma once

#include "common/common.hpp"
#include "docker_wrapper.hpp"

#include <mutex>
#include <unordered_map>
#include <vector>

namespace watchman {
namespace detail {

struct Container {
    enum class Type { Python, Rust };

    std::string id;
    Type type;
    bool isReserved{false};
};

class ContainerController {
public:
    explicit ContainerController(std::string const & host);

private:
    std::unordered_map<Container::Type, std::vector<Container>> m_containers;
    std::unordered_map<Container::Type, std::string> m_containerTypeToImage;
    std::unordered_map<Container::Type, size_t> m_containerTypeToMinContainers;

    std::mutex m_mutex;
    DockerWrapper m_docker;

    void readConfig();
};
}  // namespace detail

class Service {
public:
    explicit Service(std::string const & host = kDefaultHost);
    Service(Service const &) = delete;
    Service & operator=(Service const &) = delete;

    Response runTask(RunTaskParams const & runTaskParams);

private:
    detail::ContainerController m_containerController;
};
}  // namespace watchman
