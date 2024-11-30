#pragma once

#include "common/config.hpp"
#include "project.hpp"

namespace watchman {

struct RunCodeParams {
    TaskLauncherType
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
    TaskLauncherType containerType;
    std::string userCmdLineArgs;  // args for command line while running code
    std::string pathToMainFile;
    Practice practice;
};

}  // namespace watchman
