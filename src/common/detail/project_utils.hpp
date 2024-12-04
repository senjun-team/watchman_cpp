#pragma once
#include "common/project.hpp"

namespace watchman::detail {

struct File {
    std::string name;
    std::string content;
    bool isMain = false;
};

struct Directory {
    std::string name;
    std::vector<File> files;
    std::vector<Directory> directories;
};

std::vector<PathContent> getPathsToFiles(Directory const & directory);
std::string getMainFile(std::vector<PathContent> const & pathContents);

std::string makeProjectTar(Project const & project);
Directory jsonToDirectory(std::string const & json);

}  // namespace watchman::detail
