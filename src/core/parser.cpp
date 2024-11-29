#include "parser.hpp"

#include "common/common.hpp"
#include "common/detail/project_utils.hpp"
#include "common/logging.hpp"
#include "common/project.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <unifex/execution_policy.hpp>
#include <unifex/task.hpp>

namespace watchman {

size_t constexpr kDockerTimeoutCode = 124;
size_t constexpr kDockerMemoryKill = 137;

// Required json fields
std::string const kContainerType = "container_type";
std::string const kCourseId = "course_id";
std::string const kSourceRun = "source_run";
std::string const kUserCmdLineArgs = "user_cmd_line_args";
std::string const kPracticeAction = "action";
std::string const kProjectId = "project_id";

// Reads rapid json array to std::vector
template<class T>
void getArray(rapidjson::Value const & src, std::vector<T> & dst) {
    dst.reserve(src.Size());

    for (auto it = src.Begin(); it != src.End(); ++it) {
        auto value = it->template Get<T>();
        dst.emplace_back(std::move(value));
    }
}

/* python request sample
 curl -X 'POST' \
    'http://127.0.0.1:8000/check' \
    -H 'accept: application/json' \
    -H 'Content-Type: application/json' \
    -d '{
    "container_type": "python",
    "source_run": "err_service_unavaliable = 503",
    "source_test": "try:\n    err_service_unavaliable = 503\n\n    if err_service_unavailable !=
 503:\n        print(\"Variable value is not 503\")\n        exit(1)\n\n    if
 type(err_service_unavailable) is not int:\n        print(\"Variable is not an integer\")\n
 exit(1)\n\nexcept Exception:\n    print(\"There is no err_service_unavailable variable\")\n
 exit(1)"
 }'
 */

/* rust request sample
curl -X 'POST' \
   'http://127.0.0.1:8000/check' \
   -H 'accept: application/json' \
   -H 'Content-Type: application/json' \
   -d '{
   "container_type": "rust",
   "source_run": "fn main() {println!(\"Hello, world!\");}",
   "source_test": "",
   "cmd_line_args": ["-c never", "-j 4"]
}'
*/

class DocumentKeeper {
public:
    explicit DocumentKeeper(std::string const & body)
        : m_body(body) {
        if (m_document.Parse(m_body).HasParseError()) {
            Log::error("Incoming json has parse error: {}", m_body);
            throw std::runtime_error{"Incorrect input json"};
        }
    }

    std::string getString(std::string const & member) const {
        return m_document[member].GetString();
    }

    rapidjson::Value const & getArray(std::string const & member) const {
        return m_document[member].GetArray();
    }

    bool hasField(std::string const & member, bool required = true) const {
        if (!m_document.HasMember(member)) {
            if (required) {
                Log::error("Incoming json has no member \'{}\', json: {}", member, m_body);
            }
            return false;
        }
        return true;
    }

    bool isString(std::string const & member) const { return m_document[member].IsString(); }

    bool isArray(std::string const & member) const { return m_document[member].IsArray(); }

    bool requiredFieldIsOk(std::string const & member) const {
        return hasField(member) && isString(member);
    }

    bool requiredFieldsAreOk(std::vector<std::string> const & fields) const {
        for (auto const & field : fields) {
            if (!requiredFieldIsOk(field)) {
                Log::info("Required field is absnet: {}", field);
                return false;
            }
        }
        return true;
    }

    bool requiredFieldIsObject(std::string const & member) { return m_document[member].IsObject(); }

    std::string getProject() const { return m_document["project"].GetString(); }
    std::string getPractice() const { return m_document["project_contents"].GetString(); }
    std::string getPracticeAction() const { return m_document["action"].GetString(); }

private:
    std::string const & m_body;
    rapidjson::Document m_document;
};

std::vector<std::string> getRequiredFields(Api api) {
    switch (api) {
    case Api::Check: return {kContainerType, kSourceRun};
    case Api::Playground: return {kContainerType};
    case Api::Practice: return {};
    }

    // never go here
    throw std::logic_error{"getRequiredFields logic error"};
}

std::string getContainerTypeString(Api api) {
    switch (api) {
    case Api::Check:
    case Api::Playground: return kContainerType;
    case Api::Practice: return kCourseId;
    }
    throw std::logic_error{"getContainerTypeString logic error"};
}

RunCodeParams parseCommon(DocumentKeeper const & document, Api api) {
    // Optional json fields
    std::string const cmdLineArgs = "cmd_line_args";

    // required fields for all handles
    std::vector const requiredFields = getRequiredFields(api);

    for (std::string const & requiredField : requiredFields) {
        if (!document.requiredFieldIsOk(requiredField)) {
            return {};
        }
    }

    RunCodeParams params;

    params.containerType = document.getString(getContainerTypeString(api));

    if (document.hasField(cmdLineArgs, false)) {
        if (!document.isArray(cmdLineArgs)) {
            return {};
        }

        getArray(document.getArray(cmdLineArgs), params.cmdLineArgs);
    }

    return params;
}

RunTaskParams parseTask(std::string const & body) {
    std::string const sourceTest = "source_test";

    DocumentKeeper document(body);
    if (!document.requiredFieldIsOk(sourceTest)) {
        return {};
    }

    RunCodeParams codeParams = parseCommon(document, Api::Check);

    RunTaskParams taskParams;
    taskParams.containerType = codeParams.containerType + "_check";
    taskParams.sourceRun = document.getString(kSourceRun);
    taskParams.cmdLineArgs = codeParams.cmdLineArgs;
    taskParams.sourceTest = document.getString(sourceTest);

    return taskParams;
}

Project parseProject(std::string const & json) {
    auto const directory = detail::jsonToDirectory(json);
    return {directory.name, getPathsToFiles(directory)};
}

PracticeAction getPracticeAction(std::string const & action) {
    if (action == "run") {
        return PracticeAction::Run;
    }

    return PracticeAction::Test;
}

RunProjectParams parsePlayground(std::string const & body) {
    std::string const project = "project";

    DocumentKeeper document(body);
    if (!document.requiredFieldIsOk(project)) {
        return {};
    }

    RunCodeParams codeParams = parseCommon(document, Api::Playground);
    codeParams.containerType += "_playground";  // todo make it better?

    RunProjectParams projectParams;
    projectParams.containerType = codeParams.containerType;
    projectParams.cmdLineArgs = codeParams.cmdLineArgs;
    projectParams.project = parseProject(document.getProject());

    return projectParams;
}

RunPracticeParams parsePractice(std::string const & body) {
    std::string const project_contents = "project_contents";

    DocumentKeeper document(body);
    if (!document.requiredFieldsAreOk(
            {project_contents, kCourseId, kPracticeAction, kUserCmdLineArgs, kProjectId})) {
        return {};
    }

    RunPracticeParams params;
    params.containerType = document.getString(getContainerTypeString(Api::Practice)) + "_practice";
    params.practice.project = parseProject(document.getPractice());
    params.practice.action = getPracticeAction(document.getPracticeAction());
    params.userCmdLineArgs = document.getString(kUserCmdLineArgs);
    params.pathToMainFile = detail::getMainFile(params.practice.project.pathsContents);

    return params;
}

std::string makeJsonCourse(Response && response) {
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer writer(stringBuffer);

    writer.StartObject();
    writer.Key("status_code");
    writer.Int64(response.sourceCode);

    if (response.sourceCode == kDockerTimeoutCode) {
        writer.Key("user_code_output");
        writer.String("Timeout");
        writer.EndObject();
        return stringBuffer.GetString();
    }

    if (response.sourceCode == kDockerMemoryKill) {
        writer.Key("user_code_output");
        writer.String("Out of memory");
        writer.EndObject();
        return stringBuffer.GetString();
    }

    writer.Key("user_code_output");
    writer.String(response.output);

    if (response.testsOutput.has_value()) {
        writer.Key("tests_output");
        writer.String(*response.testsOutput);
    }

    writer.EndObject();
    return stringBuffer.GetString();
}

std::string makeJsonPlayground(Response && response) { return makeJsonCourse(std::move(response)); }

std::string makeErrorJson(std::string const & message) {
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer writer(stringBuffer);

    writer.StartObject();
    writer.Key("output");
    writer.String(message);
    writer.EndObject();
    return stringBuffer.GetString();
}

}  // namespace watchman
