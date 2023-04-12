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

    DockerWrapper dockerWrapper;
    std::string id;
    Type type{Type::Unknown};
    bool isReserved{false};

    Container(std::string host, std::string id, Type type);
    DockerAnswer runCode(std::string const & code);
    DockerAnswer clean();
};

struct Language {
    std::string imageName;
    uint32_t launched{0};
};

struct Config {
    uint32_t maxContainersAmount{0};
    std::unordered_map<Container::Type, Language> languages;
};

class ConfigParser {
public:
    explicit ConfigParser(std::string_view configPath);
    std::unordered_map<Container::Type, Language> getLanguages() const;

private:
    template<typename Ptree>
    void fillConfig(Ptree const & root);

    Config m_config;
};

class ContainerController {
public:
    ContainerController(std::string host, std::string_view configPath);
    ~ContainerController();

    ContainerController(ContainerController const & other) = delete;
    ContainerController(ContainerController && other) = delete;
    ContainerController & operator=(ContainerController const & other) = delete;
    ContainerController & operator=(ContainerController && other) = delete;

    Container & getReadyContainer(Container::Type type);
    void containerReleased(Container & container);

private:
    std::unordered_map<Container::Type, std::vector<std::shared_ptr<Container>>> m_containers;

    std::mutex m_mutex;
    std::condition_variable m_containerFree;

    detail::ConfigParser m_config;
    std::string m_dockerHost;

    void killOldContainers(DockerWrapper & dockerWrapper,
                           std::unordered_map<Container::Type, Language> const & languages);
    void launchNewContainers(DockerWrapper & dockerWrapper,
                             std::unordered_map<Container::Type, Language> const & languages);
};
}  // namespace detail

class Service {
public:
    Service(std::string const & host, std::string_view configPath);
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
