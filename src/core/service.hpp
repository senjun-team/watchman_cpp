#pragma once

#include "common/common.hpp"
#include "docker_wrapper.hpp"

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace watchman {
namespace detail {

struct Container {
    enum class Type { Python, Rust, Unknown };

    struct DockerAnswer {
        ErrorCode code{kInvalidCode};
        std::string output;

        bool isValid() const;
    };

    DockerWrapper & dockerWrapper;
    std::string id;
    Type type{Type::Unknown};
    bool isReserved{false};

    Container(DockerWrapper & dockerWrapper, std::string id, Type type);
    DockerAnswer runCode(std::string const & code);
    DockerAnswer clean();
};

class ContainerController {
public:
    explicit ContainerController(std::string host);
    ~ContainerController();

    ContainerController(ContainerController const & other) = delete;
    ContainerController(ContainerController && other) = delete;
    ContainerController & operator=(ContainerController const & other) = delete;
    ContainerController & operator=(ContainerController && other) = delete;

    Container & getReadyContainer(Container::Type type);
    void containerReleased(Container & container);

private:
    std::unordered_map<Container::Type, std::string_view> m_containerTypeToImage;
    std::unordered_map<Container::Type, size_t> m_containerTypeToMinContainers;
    std::unordered_map<Container::Type, std::vector<Container>> m_containers;

    std::mutex m_mutex;
    std::condition_variable m_containerFree;

    DockerWrapper m_dockerWrapper;
    void readConfig();
};
}  // namespace detail

class Service {
public:
    Service();
    explicit Service(std::string host);
    ~Service() = default;

    Service(Service const &) = delete;
    Service(Service &&) = delete;

    Service & operator=(Service const &) = delete;
    Service & operator=(Service &&) = delete;

    Response runTask(RunTaskParams const & runTaskParams);

private:
    static detail::Container::Type getContainerType(std::string const & type);
    detail::Container & getReadyContainer(detail::Container::Type type);

    detail::ContainerController m_containerController;
};
}  // namespace watchman
