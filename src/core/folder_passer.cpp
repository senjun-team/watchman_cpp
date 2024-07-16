#include "common/common.hpp"
#include "common/logging.hpp"
#include "parser.hpp"

#include <rapidjson/document.h>

namespace {

std::string const name = "name";
std::string const children = "children";
std::string const contents = "contents";

bool isFile(auto && child) { return child.HasMember(contents); };

bool isFolder(auto && child) { return child.HasMember(children); };

void recursiveFolderPass(watchman::Folder & folder, auto && document) {
    if (!document.HasMember(name)) {
        watchman::Log::warning("Folder without name");
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
            recursiveFolderPass(subFolder, child);
        }
    }
}
}  // namespace

namespace watchman {

Folder jsonToFolder(std::string const & json) {
    rapidjson::Document document;
    if (document.Parse(json).HasParseError()) {
        Log::error("Incoming json has parse error: {}", json);
        return {};
    }

    Folder folder;
    recursiveFolderPass(folder, document);
    return folder;
}
}  // namespace watchman
