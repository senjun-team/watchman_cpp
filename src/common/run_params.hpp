#pragma once

#include "common/api.hpp"
#include "project.hpp"

namespace watchman {

struct RequiredParams {
    Language language;
    std::vector<std::string> cmdLineArgs;  // args for launching scripts inside container
};

struct CourseTaskParams : RequiredParams {
    std::string sourceRun;
    std::string sourceTest;
};

struct PlaygroundTaskParams : RequiredParams {
    Project project;
};

// TODO
// rethink this structure
// it should be like courses/playground
struct RunPracticeParams {
    Language language;
    std::string userCmdLineArgs;  // args for command line while running code
    std::string pathToMainFile;
    Practice practice;
};

}  // namespace watchman
