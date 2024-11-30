#pragma once

#include <string>
#include <vector>

namespace watchman {

struct PathContent {
    std::string path;
    std::string content;
    bool isMain = false;
    bool isDir = false; // isDir == false means file
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

}  // namespace watchman
