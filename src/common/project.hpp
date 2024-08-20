#pragma once

#include <string>
#include <vector>

namespace watchman {

struct File {
    std::string name;
    std::string content;
};

struct Directory {
    std::string name;
    std::vector<File> files;
    std::vector<Directory> directories;
};

struct PathContent {
    std::string path;
    std::string content;
};

struct Project {
    std::string name;
    std::vector<PathContent> pathsContents;
};

std::vector<PathContent> getPathsToFiles(Directory const & directory);

std::ostringstream makeProjectTar(Project const & project);
Directory jsonToDirectory(std::string const & json);

}  // namespace watchman
