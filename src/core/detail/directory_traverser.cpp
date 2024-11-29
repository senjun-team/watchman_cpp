#include "common/detail/project_utils.hpp"
#include "common/logging.hpp"

#include <rapidjson/document.h>

#include <common/project.hpp>

namespace watchman::detail {

std::string const name = "name";
std::string const children = "children";
std::string const contents = "contents";
std::string const isMainFile = "is_main_file";

bool isFile(auto && child) { return child.HasMember(contents); };

bool isDirectory(auto && child) { return child.HasMember(children); };

void recursiveDirectoryTraverse(watchman::detail::Directory & directory, auto && document) {
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
            watchman::detail::File file;
            file.name = child[name].GetString();
            std::string content = child[contents].GetString();
            file.content = content.empty()
                             ? "\n"
                             : std::move(content);  // TODO crutch, we can't put empty files to tar
            if (child.HasMember(isMainFile) && child[isMainFile].GetBool()) {
                file.isMain = true;
            }

            directory.files.push_back(std::move(file));
            continue;
        }

        if (isDirectory(child)) {
            auto & subDirectory = directory.directories.emplace_back();
            recursiveDirectoryTraverse(subDirectory, child);
        }
    }
}

Directory jsonToDirectory(std::string const & json) {
    rapidjson::Document document;
    if (document.Parse(json).HasParseError()) {
        Log::info("Json has parse error at offest: {}", document.GetErrorOffset());

        throw std::runtime_error{
            fmt::format("Project tree has sytax error in the end of string: {}",
                        std::string_view{json.data(), document.GetErrorOffset()})};
    }

    Directory directory;
    recursiveDirectoryTraverse(directory, document);
    return directory;
}
}  // namespace watchman::detail
