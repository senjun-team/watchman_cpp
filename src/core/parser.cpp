#include "parser.hpp"

#include "common/logging.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace watchman {

size_t constexpr kDockerTimeoutCode = 124;
size_t constexpr kDockerMemoryKill = 137;

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

RunTaskParams parse(std::string const & body, Api api) {
    // Required json fields
    std::string const containerType = "container_type";
    std::string const sourceRun = "source_run";
    std::string const sourceTest = "source_test";

    // Optional json fields
    std::string const cmdLineArgs = "cmd_line_args";

    rapidjson::Document document;
    if (document.Parse(body).HasParseError()) {
        Log::error("Incoming json has parse error: {}", body);
        return {};
    }

    auto const hasField = [&document, &body](std::string const & member,
                                             bool isRequired = true) -> bool {
        if (!document.HasMember(member)) {
            if (isRequired) {
                Log::error("Incoming json has no member \'{}\', json: {}", member, body);
            }
            return false;
        }
        return true;
    };

    auto const isString = [&document, &body](std::string const & member) -> bool {
        if (!document[member].IsString()) {
            Log::error("Incoming json has member \'{}\' which is not a string, json: {}", member,
                       body);
            return false;
        }
        return true;
    };

    auto const getString = [&document](std::string const & member) -> std::string {
        return document[member].GetString();
    };

    auto const requiredFieldIsOk = [&hasField, &isString](std::string const & member) -> bool {
        if (!hasField(member) || !isString(member)) {
            return false;
        }
        return true;
    };

    // required fields for all handles
    std::vector const requiredFields{containerType, sourceRun};

    for (std::string const & requiredField : requiredFields) {
        if (!requiredFieldIsOk(requiredField)) {
            return {};
        }
    }

    RunTaskParams params;
    // todo change name of container depend on api
    std::string const suffix = api == Api::Check ? "_check" : "_playground";
    params.containerType = getString(containerType) + suffix;
    params.sourceRun = getString(sourceRun);

    if (api == Api::Check) {
        if (!requiredFieldIsOk(sourceTest)) {
            return {};
        }
        params.sourceTest = getString(sourceTest);
    }

    if (hasField(cmdLineArgs, false)) {
        if (!document[cmdLineArgs].IsArray()) {
            return {};
        }

        getArray(document[cmdLineArgs].GetArray(), params.cmdLineArgs);
    }

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

// folder stuff

std::string const name = "name";
std::string const children = "children";
std::string const contents = "contents";

bool isFile(auto && child) { return child.HasMember(contents); };

bool isFolder(auto && child) { return child.HasMember(children); };

void recursivePass(Folder & folder, auto && document) {
    if (!document.HasMember(name)) {
        Log::warning("Folder without name");
        return;
    }
    folder.name = document[name].GetString();
    if (!document.HasMember(children) || !document[children].IsArray()) {
        return;
    }

    auto const & childrenArray = document[children].GetArray();
    for (auto const & child : childrenArray) {
        if (isFile(child)) {
            folder.files.push_back({child[name].GetString(), child[contents].GetString()});
            continue;
        }

        if (isFolder(child)) {
            auto & subFolder = folder.folders.emplace_back();
            subFolder.name = child[name].GetString();
            recursivePass(subFolder, child);
        }
    }
};

Folder fillFolder(std::string const & json) {
    rapidjson::Document document;
    if (document.Parse(json).HasParseError()) {
        Log::error("Incoming json has parse error: {}", json);
        return {};
    }

    Folder folder;
    recursivePass(folder, document);
    return folder;
}

}  // namespace watchman
