#pragma once

#include "project.hpp"

namespace watchman {

struct RunCodeParams {
    std::string
        containerType;  // language with suffix, e.g. python_check, cpp_practice, go_playground
    std::vector<std::string> cmdLineArgs;  // args for launching scripts inside container
};

struct RunTaskParams : RunCodeParams {
    std::string sourceRun;
    std::string sourceTest;
};

struct RunProjectParams : RunCodeParams {
    Project project;
};

struct RunPracticeParams {
    std::string containerType;
    std::string userCmdLineArgs;  // args for command line while running code
    std::string pathToMainFile;
    Practice practice;
};

}  // namespace watchman
