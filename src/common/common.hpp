#pragma once

#include "project.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace watchman {

std::string const kFilenameTask = "task";
std::string const kFilenameTaskTests = "task_tests";
using ErrorCode = int32_t;
static ErrorCode constexpr kSuccessCode = 0;
static ErrorCode constexpr kUserCodeError = 1;
static ErrorCode constexpr kTestsError = 2;
static ErrorCode constexpr kInvalidCode = -1;

bool errorCodeIsUnexpected(ErrorCode code);

enum class Api { Check, Playground };

struct RunCodeParams {
    std::string containerType;
    std::vector<std::string> cmdLineArgs;
};

struct RunTaskParams : RunCodeParams {
    std::string sourceRun;
    std::string sourceTest;
};

struct RunProjectParams : RunCodeParams {
    Project project;
};

struct Response {
    ErrorCode sourceCode{kInvalidCode};
    std::string output;
    std::optional<std::string> testsOutput;
};

struct CointainerTypeInfo {
    std::string imageName;
    uint32_t launched{0};
};

using Language = CointainerTypeInfo;
using Playground = CointainerTypeInfo;

struct Config {
    using ContainerType = std::string;  // python/rust/go/haskell
    std::optional<size_t> threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<ContainerType, Language> languages;
    std::unordered_map<ContainerType, Playground> playgrounds;
};

size_t getCpuCount();

Config readConfig(std::string_view configPath);

struct CodeFilename {
    std::string code;
    std::string filename;
};

std::ostringstream makeTar(std::vector<CodeFilename> && data);

class LogDuration {
public:
    LogDuration(std::string operation);
    ~LogDuration();

private:
    std::string const m_operation;
    std::chrono::system_clock::time_point m_start;
};

namespace detail {
class BaseContainer;

struct ProtectedContainers {
    std::mutex mutex;
    std::condition_variable containerFree;
    std::unordered_map<Config::ContainerType, std::vector<std::shared_ptr<BaseContainer>>>
        containers;
};
}  // namespace detail

}  // namespace watchman
