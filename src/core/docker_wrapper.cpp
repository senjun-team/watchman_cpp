#include "docker_wrapper.hpp"

namespace watchman {

DockerWrapper::DockerWrapper(std::string const & host)
    : m_docker(host)
    , m_writer(m_stringBuffer) {}

std::vector<Container> DockerWrapper::getAllContainers() {
    auto const response = m_docker.list_containers(true);
    auto const & dataValue = response["data"];
    if (!response["success"].GetBool() || !dataValue.IsArray()) {
        return {};
    }

    auto const rawContainers = dataValue.GetArray();
    size_t const size = rawContainers.Size();

    std::vector<Container> containers;
    containers.reserve(size);
    for (size_t index = 0; index < size; ++index) {
        if (!rawContainers[index].HasMember("Id") || !rawContainers[index].HasMember("Image")) {
            continue;
        }

        containers.push_back(
            {rawContainers[index]["Id"].GetString(), rawContainers[index]["Image"].GetString()});
    }

    return containers;
}

bool DockerWrapper::isRunning(std::string const & id) {
    auto const response = m_docker.inspect_container(id);
    auto const & dataValue = response["data"];
    if (!response["success"].GetBool() || !dataValue.IsObject()
        || !dataValue.GetObject().HasMember("State") || !dataValue.GetObject()["State"].IsObject()
        || !dataValue.GetObject()["State"].GetObject().HasMember("Running")) {
        return false;
    }

    return dataValue.GetObject()["State"].GetObject()["Running"].GetBool();
}

std::string DockerWrapper::getImage(std::string const & id) {
    auto const response = m_docker.inspect_container(id);
    auto const & dataValue = response["data"];
    if (!response["success"].GetBool() || !dataValue.IsObject()
        || !dataValue.GetObject().HasMember("Config") || !dataValue.GetObject()["Config"].IsObject()
        || !dataValue.GetObject()["Config"].GetObject().HasMember("Image")) {
        return {};
    }

    return dataValue.GetObject()["Config"].GetObject()["Image"].GetString();
}

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

bool DockerWrapper::putArchive(DockerPutArchiveParams && params) {
    auto const result =
        m_docker.put_archive(params.containerId, params.pathInContainer, params.pathInContainer);
    return result["success"].GetBool();
}

DockerExecResult DockerWrapper::exec(DockerExecParams && params) {
    std::string const execRequest = makeJsonHelper().getExecParams(std::move(params.command));
    std::string const execStartRequest = makeJsonHelper().getExecStartParams();

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

detail::JsonHelperInitializer DockerWrapper::makeInitializer() {
    return {m_stringBuffer, m_writer};
}

detail::JsonHelper DockerWrapper::makeJsonHelper() { return {makeInitializer()}; }

detail::JsonHelper::JsonHelper(detail::JsonHelperInitializer const & initializer)
    : m_stringBuffer(initializer.stringBuffer)
    , m_writer(initializer.writer) {
    m_writer.StartObject();
}

detail::JsonHelper::~JsonHelper() {
    m_stringBuffer.Clear();
    m_writer.Reset(m_stringBuffer);
}

std::string detail::JsonHelper::getRunRequest(DockerRunParams && params) && {
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

std::string detail::JsonHelper::getExecParams(std::vector<std::string> && command) && {
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
std::string detail::JsonHelper::getExecStartParams() && {
    // TODO may be should improve
    return R"({"Detach": false, "Tty": false})";
}
}  // namespace watchman
