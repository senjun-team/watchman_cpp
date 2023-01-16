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
    rapidjson::Document document;
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

DockerExecResult DockerWrapper::exec(DockerExecParams && params) const { return {}; }

void DockerWrapper::putArchive(DockerPutArchiveParams && params) const {}

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

    if (params.tty) {
        m_writer.Key("Tty");
        m_writer.Bool(params.tty.value());
    }

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
}  // namespace watchman
