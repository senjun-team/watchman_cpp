#pragma once

#include "../common/common.hpp"
#include "docker_wrapper.hpp"

#include <mutex>
#include <unordered_map>
#include <vector>

namespace watchman {
class Service {
public:
    Service();
    explicit Service(std::string const & host);

    Response runTask(RunTaskParams const & runTaskParams);

private:
    class ContainerController {
    public:
        ContainerController();
        explicit ContainerController(std::string const & host);

    private:
        struct Container {
            enum class Type { Python, Rust };

            std::string const id;
            Type const type;
            bool isReserved{false};
        };

        std::unordered_map<Container::Type, std::vector<Container>> m_containers;
        std::unordered_map<Container::Type, std::string> m_containerTypeToImage;
        std::unordered_map<Container::Type, size_t> m_containerTypeToMinContainers;

        std::mutex m_mutex;
        DockerWrapper m_docker;

        void readConfig();
    };

    ContainerController m_containerController;
};
}  // namespace watchman
