#include "docker_wrapper.hpp"

namespace watchman {

DockerWrapper::DockerWrapper()
    : m_writer(m_stringBuffer) {}

DockerWrapper::DockerWrapper(std::string const & host)
    : m_docker(host)
    , m_writer(m_stringBuffer) {}

std::vector<std::string> DockerWrapper::getAllContainers() const { return {}; }

bool DockerWrapper::isRunning(std::string const & id) const { return false; }

std::string DockerWrapper::getImage(std::string const & id) const { return {}; }

bool DockerWrapper::killContainer(std::string const & id) {
    if (id.empty()) {
        // log
        return false;
    }

    auto const responseKill = m_docker.kill_container(id);
    if (!responseKill["success"].GetBool()) {
        // log
        return false;
    }

    return true;
}

bool DockerWrapper::removeContainer(std::string const & id) {
    if (id.empty()) {
        // log
        return false;
    }

    auto const responseKill = m_docker.delete_container(id);
    if (!responseKill["success"].GetBool()) {
        // log
        return false;
    }

    return true;
}

std::string DockerWrapper::run(DockerRunParams && params) {
    std::string const request = makeJsonHelper().getRunRequest(std::move(params));
    JSON_DOCUMENT document;
    if (document.Parse(request).HasParseError()) {
        // log
        return {};
    }
    auto const response = m_docker.run_container(document);
    if (!response["success"].GetBool() || !response["data"].GetObject().HasMember("Id")) {
        // log
        return {};
    }

    return response["data"].GetObject()["Id"].GetString();
}

void DockerWrapper::putArchive(DockerPutArchiveParams && params) const {}

DockerExecResult DockerWrapper::exec(DockerExecParams && params) {
    std::string execRequest = makeJsonHelper().getExecParams(std::move(params.command));
    std::string execStartRequest = makeJsonHelper().getExecStartParams();

    JSON_DOCUMENT execParams;
    if (execParams.Parse(execRequest).HasParseError()) {
        return {.exitCode = -1, .output = "Parse error 1"};
    }

    JSON_DOCUMENT execStartParams;
    if (execStartParams.Parse(execStartRequest).HasParseError()) {
        return {.exitCode = -1, .output = "Parse error 2"};
    }

    auto result = m_docker.exec(execParams, execStartParams, params.containerId);
    if (!result.HasMember("success") || !result["success"].GetBool() || !result.HasMember("data")) {
        return {.exitCode = -1, .output = "docker exec error"};
    }

    return {.exitCode = 0, .output = jsonToString(result["data"])};
}

DockerWrapper::JsonHelperInitializer DockerWrapper::makeInitializer() {
    return {m_stringBuffer, m_writer};
}

DockerWrapper::JsonHelper DockerWrapper::makeJsonHelper() { return {makeInitializer()}; }

DockerWrapper::JsonHelper::JsonHelper(DockerWrapper::JsonHelperInitializer const & initializer)
    : m_stringBuffer(initializer.stringBuffer)
    , m_writer(initializer.writer) {
    m_writer.StartObject();
}

DockerWrapper::JsonHelper::~JsonHelper() {
    m_stringBuffer.Clear();
    m_writer.Reset(m_stringBuffer);
}

std::string DockerWrapper::JsonHelper::getRunRequest(DockerRunParams && params) && {
    m_writer.Key("Image");
    m_writer.String(params.image);

    m_writer.Key("Tty");
    m_writer.Bool(true);

    if (params.memoryLimit) {
        m_writer.Key("HostConfig");
        m_writer.StartObject();
        m_writer.Key("Memory");
        m_writer.Uint64(params.memoryLimit.value());
        m_writer.EndObject();
    }

    m_writer.EndObject();

    return m_stringBuffer.GetString();
}

std::string DockerWrapper::JsonHelper::getExecParams(std::vector<std::string> && command) && {
    m_writer.Key("AttachStderr");
    m_writer.Bool(true);

    m_writer.Key("AttachStdout");
    m_writer.Bool(true);

    m_writer.Key("Detach");
    m_writer.Bool(false);

    m_writer.Key("Tty");
    m_writer.Bool(true);

    m_writer.Key("Cmd");
    m_writer.StartArray();
    for (size_t index = 0; index < command.size(); ++index) {
        m_writer.String(std::move(command[index]));
    }
    m_writer.EndArray();
    m_writer.EndObject();

    return m_stringBuffer.GetString();
}
std::string DockerWrapper::JsonHelper::getExecStartParams() && {
    // TODO may be should improve
    return R"({"Detach": false, "Tty": false})";
}
}  // namespace watchman
