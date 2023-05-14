#pragma once

#include <chrono>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace watchman {
std::string const kFilenameTask = "task";
std::string const kFilenameTaskTests = "task_tests";
using ErrorCode = int32_t;
static ErrorCode constexpr kSuccessCode = 0;
static ErrorCode constexpr kInvalidCode = -1;

enum class ContainerType { Python, Rust, Unknown };

struct RunTaskParams {
    std::string containerType;
    std::string sourceRun;
    std::string sourceTest;
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
    std::optional<size_t> threadPoolSize;
    uint32_t maxContainersAmount{0};
    std::unordered_map<ContainerType, Language> languages;
};

size_t getCpuCount();
ContainerType getContainerType(std::string const & type);

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
