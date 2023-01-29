#pragma once
#include <docker.hpp>
#include <optional>
#include <string>
#include <vector>

namespace watchman {

struct DockerRunParams {
    std::string image;
    std::optional<bool> tty;
    std::optional<size_t> memoryLimit;  // in bytes
};

struct DockerExecResult {
    int32_t exitCode{0};
    std::string output;
};

struct DockerExecParams {
    std::vector<std::string> command;
    std::string containerId;
};

struct DockerPutArchiveParams {
    std::string containerId;
    std::string pathInContainer;
    std::string pathToArchive;
};

struct Container {
    std::string id;
    std::string image;
};

namespace detail {

struct JsonHelperInitializer {
    rapidjson::StringBuffer & stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> & writer;
};

class JsonHelper {
public:
    JsonHelper(JsonHelperInitializer const & initializer);
    ~JsonHelper();

    std::string getRunRequest(DockerRunParams && params) &&;
    std::string getExecParams(std::vector<std::string> && command) &&;
    std::string getExecStartParams() &&;

private:
    rapidjson::StringBuffer & m_stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> & m_writer;
};

}  // namespace detail

class DockerWrapper {
public:
    explicit DockerWrapper(std::string const & host = kDefaultHost);

    DockerWrapper(DockerWrapper const &) = delete;
    DockerWrapper & operator=(DockerWrapper const &) = delete;

    std::string run(DockerRunParams && params);
    DockerExecResult exec(DockerExecParams && params);
    std::vector<Container> getAllContainers();
    bool isRunning(std::string const & id);
    std::string getImage(std::string const & id);
    bool killContainer(std::string const & id);
    bool removeContainer(std::string const & id);
    bool putArchive(DockerPutArchiveParams && params);

private:
    Docker m_docker;
    rapidjson::StringBuffer m_stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> m_writer;

    enum class DataType;
    static bool isAnswerCorrect(JSON_DOCUMENT const & document, DataType type);

    detail::JsonHelperInitializer makeInitializer();
    detail::JsonHelper makeJsonHelper();
};
}  // namespace watchman
