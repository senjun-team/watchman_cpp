#pragma once

#include <chrono>
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
static ErrorCode constexpr kInvalidCode = -1;

struct RunTaskParams {
    std::string containerType;
    std::string sourceRun;
    std::string sourceTest;

    // Command-line arguments for interpreter
    std::vector<std::string> cmdLineArgs;
};

struct Response {
    ErrorCode sourceCode{kInvalidCode};
    ErrorCode testsCode{kInvalidCode};
    std::string output;
    std::string testsOutput;
};

struct Language {
    std::string imageName;
    uint32_t launched{0};
};

struct Config {
    using ContainerType = std::string;
    std::optional<size_t> threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<ContainerType, Language> languages;
};

size_t getCpuCount();

Config readConfig(std::string_view configPath);
std::ostringstream makeTar(std::string const & sourceCode, std::string const & sourceTests);

class LogDuration {
public:
    LogDuration(std::string operation);
    ~LogDuration();

private:
    std::string const m_operation;
    std::chrono::system_clock::time_point m_start;
};
}  // namespace watchman
