#pragma once

#include "common/config.hpp"
#include "project.hpp"

#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>
#include <string_view>

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

struct BaseCodeLauncher;
namespace detail {

struct CodeLaunchersStorage {
    std::mutex mutex;
    std::condition_variable codeLauncherFree;
    std::unordered_map<Config::CodeLauncherType, std::list<std::unique_ptr<BaseCodeLauncher>>>
        codeLaunchers;
};
}  // namespace detail

}  // namespace watchman
