#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <sstream>

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
};

struct Response {
    ErrorCode sourceCode{kInvalidCode};
    ErrorCode testsCode{kInvalidCode};
    std::string output;
    std::string testsOutput;
};

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
