#include "parser.hpp"

#include "common/logging.hpp"

#include <rapidjson/document.h>

namespace watchman {

// request sample
// curl -X 'POST' \
//    'http://127.0.0.1:8000/check' \
//    -H 'accept: application/json' \
//    -H 'Content-Type: application/json' \
//    -d '{
//    "container_type": "python",
//    "source_run": "print(42)",
//    "source_test": "print(69)"
//}'

RunTaskParams parse(std::string const & body) {
    RunTaskParams const fields{
        .containerType = "container_type", .sourceRun = "source_run", .sourceTest = "source_test"};

    rapidjson::Document document;
    if (document.Parse(body).HasParseError()) {
        Log::error("Incoming json has parse error: {}", body);
        return {};
    }

    auto const hasField = [&document, &body](std::string const & member) -> bool {
        if (!document.HasMember(member)) {
            Log::error("Incoming json has no member \'{}\', json: {}", member, body);
            return false;
        }

        if (!document[member].IsString()) {
            Log::error("Incoming json has member \'{}\' but it is not a string, json: {}", member,
                       body);
            return false;
        }
        return true;
    };

    if (!hasField(fields.containerType) || !hasField(fields.sourceRun)) {
        return {};
    }

    auto const getField = [&document](std::string const & member) -> std::string {
        return document[member].GetString();
    };

    return {.containerType = getField(fields.containerType),
            .sourceRun = getField(fields.sourceRun),
            .sourceTest = hasField(fields.sourceTest.value())
                            ? std::make_optional(getField(fields.sourceTest.value()))
                            : std::nullopt};
}

}  // namespace watchman
