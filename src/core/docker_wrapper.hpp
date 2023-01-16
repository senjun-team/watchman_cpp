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

class DockerWrapper {
public:
    DockerWrapper();
    explicit DockerWrapper(std::string const & host);

    DockerWrapper(DockerWrapper const &) = delete;
    DockerWrapper & operator=(DockerWrapper const &) = delete;

    std::string run(DockerRunParams && params);
    DockerExecResult exec(DockerExecParams && params);
    std::vector<std::string> getAllContainers() const;
    bool isRunning(std::string const & id) const;
    std::string getImage(std::string const & id) const;
    bool killContainer(std::string const & id);
    bool removeContainer(std::string const & id);
    void putArchive(DockerPutArchiveParams && params) const;

private:
    Docker m_docker;
    rapidjson::StringBuffer m_stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> m_writer;

    struct JsonHelperInitializer {
        rapidjson::StringBuffer & stringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> & writer;
    };

    JsonHelperInitializer makeInitializer();

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

    JsonHelper makeJsonHelper();
};
}  // namespace watchman
