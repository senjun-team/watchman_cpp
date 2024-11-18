#pragma once

#include "project.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <list>

namespace watchman {

std::string const kFilenameTask = "task";
std::string const kFilenameTaskTests = "task_tests";
using ErrorCode = int32_t;
static ErrorCode constexpr kSuccessCode = 0;
static ErrorCode constexpr kUserCodeError = 1;
static ErrorCode constexpr kTestsError = 2;
static ErrorCode constexpr kInvalidCode = -1;

bool errorCodeIsUnexpected(ErrorCode code);

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

struct Response {
    ErrorCode sourceCode{kInvalidCode};
    std::string output;
    std::optional<std::string> testsOutput;
};

struct ContainerTypeInfo {
    std::string imageName;
    uint32_t launched{0};
};

using Language = ContainerTypeInfo;
using Playground = ContainerTypeInfo;
using PracticeContainer = ContainerTypeInfo;

struct Config {
    using CodeLauncherType = std::string;  // python/rust/go/haskell
    std::optional<size_t> threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<CodeLauncherType, Language> languages;
    std::unordered_map<CodeLauncherType, Playground> playgrounds;
    std::unordered_map<CodeLauncherType, PracticeContainer> practices;
};

size_t getCpuCount();

Config readConfig(std::string_view configPath);

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

namespace detail {
struct BaseCodeLauncher;

struct ProtectedContainers {
    std::mutex mutex;
    std::condition_variable containerFree;
    std::unordered_map<Config::CodeLauncherType, std::list<std::unique_ptr<BaseCodeLauncher>>>
        codeLaunchers;
};
}  // namespace detail

}  // namespace watchman
