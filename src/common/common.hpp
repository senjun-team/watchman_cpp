#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace watchman {

using ErrorCode = int32_t;
static ErrorCode constexpr kSuccessCode = 0;
static ErrorCode constexpr kInvalidCode = -1;

struct RunTaskParams {
    std::string containerType;
    std::string sourceRun;
    std::optional<std::string> sourceTest;
};

struct Response {
    ErrorCode sourceCode{kInvalidCode};
    ErrorCode testsCode{kInvalidCode};
    std::string output;
};

// archiveName must have got suffix ".tar"
bool makeTar(std::string_view archiveName, std::string_view sourceCode);

class LogDuration {
public:
    LogDuration(std::string operation);
    ~LogDuration();

private:
    std::string const m_operation;
    std::chrono::system_clock::time_point m_start;


};
}  // namespace watchman
