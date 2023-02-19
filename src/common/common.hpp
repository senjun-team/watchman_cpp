#pragma once

#include <string>

namespace watchman {

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
};

// archiveName must have got suffix ".tar"
bool makeTar(std::string_view archiveName, std::string_view sourceCode);
}  // namespace watchman
