#pragma once

#include <string>
#include <vector>

namespace watchman {

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

struct PathContent {
    std::string path;
    std::string content;
    bool isMain = false;
    bool isDir = false;
};

struct Project {
    std::string name;
    std::vector<PathContent> pathsContents;
};


enum class PracticeAction { Run, Test };

struct Practice {
    Project project;
    PracticeAction action;
};

std::vector<PathContent> getPathsToFiles(Directory const & directory);
std::string getMainFile(std::vector<PathContent> const & pathContents);

std::ostringstream makeProjectTar(Project const & project);
Directory jsonToDirectory(std::string const & json);

}  // namespace watchman
