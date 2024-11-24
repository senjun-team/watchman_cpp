#pragma once

#include "project.hpp"

#include <chrono>

namespace watchman {

std::string const kFilenameTask = "task";
std::string const kFilenameTaskTests = "task_tests";

enum class Api { Check, Playground, Practice };

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

size_t getCpuCount();

struct CodeFilename {
    std::string code;
    std::string filename;
};

std::string makeTar(std::vector<CodeFilename> && data);

class LogDuration {
public:
    LogDuration(std::string operation);
    ~LogDuration();

private:
    std::string const m_operation;
    std::chrono::system_clock::time_point m_start;
};

}  // namespace watchman
