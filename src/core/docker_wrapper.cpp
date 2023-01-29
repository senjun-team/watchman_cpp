#include "docker_wrapper.hpp"

#include "common/logging.hpp"

namespace watchman {

// there's no string_view because of operator[] inside rapidjson::Value
static std::string const kId = "Id";
static std::string const kData = "data";
static std::string const kSuccess = "success";
static std::string const kImage = "Image";
static std::string const kState = "State";
static std::string const kRunning = "Running";
static std::string const kConfig = "Config";

enum class DockerWrapper::DataType { Absent, Array, Object, String };

DockerWrapper::DockerWrapper(std::string const & host)
    : m_docker(host)
    , m_writer(m_stringBuffer) {}

std::vector<Container> DockerWrapper::getAllContainers() {
    auto const response = m_docker.list_containers(true);
    if (!isAnswerCorrect(response, DataType::Array)) {
        Log::error("getAllContainers: answer error");
        return {};
    }

    auto const rawContainers = response[kData].GetArray();
    size_t const size = rawContainers.Size();

    std::vector<Container> containers;
    containers.reserve(size);
    for (size_t index = 0; index < size; ++index) {
        if (!rawContainers[index].HasMember(kId) || !rawContainers[index].HasMember(kImage)) {
            continue;
        }

        containers.push_back(
            {rawContainers[index][kId].GetString(), rawContainers[index][kImage].GetString()});
    }

    return containers;
}

bool DockerWrapper::isRunning(std::string const & id) {
    auto const response = m_docker.inspect_container(id);
    if (!isAnswerCorrect(response, DataType::Object)) {
        Log::error("isRunning: answer error for {}", id);
        return false;
    }

    auto const & dataObject = response[kData].GetObject();
    if (!dataObject.HasMember(kState) || !dataObject[kState].IsObject()
        || !dataObject[kState].GetObject().HasMember(kRunning)
        || !dataObject[kState].GetObject()[kRunning].IsBool()) {
        Log::error("isRunning: error while getting success result for {}", id);
        return false;
    }

    return dataObject[kState].GetObject()[kRunning].GetBool();
}

std::string DockerWrapper::getImage(std::string const & id) {
    auto const response = m_docker.inspect_container(id);
    if (!isAnswerCorrect(response, DataType::Object)) {
        Log::error("getImage: answer error for {}", id);
        return {};
    }

    auto const & dataObject = response[kData].GetObject();
    if (!dataObject.HasMember(kConfig) || !dataObject[kConfig].IsObject()
        || !dataObject[kConfig].HasMember(kImage)
        || !dataObject[kConfig].GetObject()[kImage].IsString()) {
        Log::error("isRunning: error while getting image name for {}", id);
        return {};
    }

    return dataObject[kConfig].GetObject()[kImage].GetString();
}

bool DockerWrapper::killContainer(std::string const & id) {
    if (id.empty()) {
        Log::error("killContainer error: id is empty");
        return false;
    }

    auto const responseKill = m_docker.kill_container(id);
    if (!isAnswerCorrect(responseKill, DataType::Absent)) {
        Log::error("killContainer parse error for {}", id);
        return false;
    }

    return responseKill[kSuccess].GetBool();
}

bool DockerWrapper::removeContainer(std::string const & id) {
    if (id.empty()) {
        Log::error("removeContainer error: id is empty");
        return false;
    }

    auto const responseKill = m_docker.delete_container(id);
    if (!isAnswerCorrect(responseKill, DataType::Absent)) {
        Log::error("removeContainer parse error for {}", id);
        return false;
    }

    return responseKill[kSuccess].GetBool();
}

std::string DockerWrapper::run(DockerRunParams && params) {
    std::string const request = makeJsonHelper().getRunRequest(std::move(params));
    JSON_DOCUMENT document;
    assert(!document.Parse(request).HasParseError());

    auto const response = m_docker.run_container(document);
    if (!isAnswerCorrect(response, DataType::Object)) {
        Log::error("run parse error for image {}", params.image);
        return {};
    }

    return response[kData].GetObject()[kId].GetString();
}

bool DockerWrapper::putArchive(DockerPutArchiveParams && params) {
    auto const result =
        m_docker.put_archive(params.containerId, params.pathInContainer, params.pathToArchive);
    if (!isAnswerCorrect(result, DataType::Absent)) {
        Log::error("putArchive parse error for container {}", params.containerId);
        return false;
    }

    return result[kSuccess].GetBool();
}

DockerExecResult DockerWrapper::exec(DockerExecParams && params) {
    std::string const execRequest = makeJsonHelper().getExecParams(std::move(params.command));
    std::string const execStartRequest = makeJsonHelper().getExecStartParams();

    JSON_DOCUMENT execParams;
    auto const & execParamsDocument = execParams.Parse(execRequest);
    assert(!execParamsDocument.HasParseError());

    JSON_DOCUMENT execStartParams;
    auto const & execStartParamsDocument = execStartParams.Parse(execStartRequest);
    assert(!execStartParamsDocument.HasParseError());

    auto result = m_docker.exec(execParams, execStartParams, params.containerId);
    if (!isAnswerCorrect(result, DataType::String)) {
        Log::error("exec parse error for container {}", params.containerId);
        return {.exitCode = -1, .output = "docker exec error"};
    }

    return {.exitCode = 0, .output = jsonToString(result[kData])};
}

detail::JsonHelperInitializer DockerWrapper::makeInitializer() {
    return {m_stringBuffer, m_writer};
}

detail::JsonHelper DockerWrapper::makeJsonHelper() { return {makeInitializer()}; }

bool DockerWrapper::isAnswerCorrect(JSON_DOCUMENT const & document, DataType type) {
    if (document.HasParseError()) {
        Log::error("isAnswerCorrect: json parse error");
        return false;
    }

    if (!document.HasMember(kSuccess)) {
        Log::error("isAnswerCorrect: there is no \'success\' field id DockerClient answer");
        return false;
    }

    if (!document[kSuccess].GetBool()) {
        Log::error("isAnswerCorrect: \'success\' field is false");
        return false;
    }

    bool correct = true;
    switch (type) {
    case DataType::Absent: break;

    case DataType::Array:
        if (!document.HasMember(kData)) {
            Log::error("isAnswerCorrect: \'data\' field is absent");
            correct = false;
            break;
        }

        if (!document[kData].IsArray()) {
            Log::error("isAnswerCorrect: \'data\' field is not an array");
            correct = false;
            break;
        }

        break;

    case DataType::Object:
        if (!document.HasMember(kData)) {
            Log::error("isAnswerCorrect: \'data\' field is absent");
            correct = false;
            break;
        }

        if (!document[kData].IsObject()) {
            Log::error("isAnswerCorrect: \'data\' field is not an object");
            correct = false;
            break;
        }

        break;

    case DataType::String:
        if (!document.HasMember(kData)) {
            Log::error("isAnswerCorrect: \'data\' field is absent");
            correct = false;
            break;
        }

        if (!document[kData].IsString()) {
            Log::error("isAnswerCorrect: \'data\' field is not a string");
            correct = false;
            break;
        }

        break;
    }

    return correct;
}

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
    m_writer.Key(kImage);
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
