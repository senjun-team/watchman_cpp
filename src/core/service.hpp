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
        ErrorCode code;
        std::string output;

        bool isValid() const;
    };

    DockerWrapper & dockerWrapper;
    std::string id;
    Type type;
    bool isReserved{false};

    Container(DockerWrapper & dockerWrapper, std::string id, Type type);
    DockerAnswer runCode(std::string const & code);
    DockerAnswer clean();
};

class ContainerController {
public:
    explicit ContainerController(std::string host);
    ~ContainerController();

    Container & getReadyContainer(Container::Type type);
    void containerReleased();

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
    explicit Service(std::string const & host = kDefaultHost);
    Service(Service const &) = delete;
    Service & operator=(Service const &) = delete;

    Response runTask(RunTaskParams const & runTaskParams);

private:
    static detail::Container::Type getContainerType(std::string const & type);

    // TODO should be blocking or not? hell yeaaah!
    detail::Container & getReadyContainer(detail::Container::Type type);

    detail::ContainerController m_containerController;
};
}  // namespace watchman
