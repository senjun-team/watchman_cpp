#pragma once

#include <optional>
#include <string>

namespace watchman {

using ErrorCode = int32_t;
static ErrorCode constexpr kSuccessCode = 0;
static ErrorCode constexpr kUserCodeError = 1;
static ErrorCode constexpr kTestsError = 2;
static ErrorCode constexpr kInvalidCode = -1;

inline bool errorCodeIsUnexpected(ErrorCode code) {
    return code != kSuccessCode && code != kUserCodeError && code != kTestsError;
}

struct Response {
    ErrorCode sourceCode{kInvalidCode};
    std::string output;
    std::optional<std::string> testsOutput;
};

}  // namespace watchman
