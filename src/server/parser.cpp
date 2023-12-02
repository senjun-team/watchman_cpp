#include "server/parser.hpp"

#include "common/logging.hpp"

#include <rapidjson/document.h>

namespace watchman {
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

RunTaskParams parse(std::string const & body) {
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

    for (std::string const & requiredField : {containerType, sourceRun, sourceTest}) {
        if (!hasField(requiredField) || !isString(requiredField)) {
            return {};
        }
    }

    RunTaskParams params{.containerType = getString(containerType),
                         .sourceRun = getString(sourceRun),
                         .sourceTest = getString(sourceTest)};

    if (hasField(cmdLineArgs, false)) {
        if (!document[cmdLineArgs].IsArray()) {
            return {};
        }

        getArray(document[cmdLineArgs].GetArray(), params.cmdLineArgs);
    }

    return params;
}

}  // namespace watchman
