#include "common/common.hpp"
#include "common/logging.hpp"
#include "parser.hpp"

#include <rapidjson/document.h>

namespace {

std::string const name = "name";
std::string const children = "children";
std::string const contents = "contents";

bool isFile(auto && child) { return child.HasMember(contents); };

bool isDirectory(auto && child) { return child.HasMember(children); };

void recursiveDirectoryPass(watchman::Directory & directory, auto && document) {
    if (!document.HasMember(name)) {
        watchman::Log::warning("Directory without name");
        return;
    }
    directory.name = document[name].GetString();
    if (!document.HasMember(children) || !document[children].IsArray()) {
        return;
    }

    auto const & childrenArray = document[children].GetArray();
    for (auto const & child : childrenArray) {
        if (isFile(child)) {
            directory.files.push_back({child[name].GetString(), child[contents].GetString()});
            continue;
        }

        if (isDirectory(child)) {
            auto & subDirectory = directory.directories.emplace_back();
            subDirectory.name = child[name].GetString();
            recursiveDirectoryPass(subDirectory, child);
        }
    }
}
}  // namespace

namespace watchman {

Directory jsonToDirectory(std::string const & json) {
    rapidjson::Document document;
    if (document.Parse(json).HasParseError()) {
        Log::error("Incoming json has parse error: {}", json);
        return {};
    }

    Directory directory;
    recursiveDirectoryPass(directory, document);
    return directory;
}
}  // namespace watchman
