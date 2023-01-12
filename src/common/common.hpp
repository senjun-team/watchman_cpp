#pragma once

#include <string>

namespace watchman {

using ErrorCode = int32_t;

struct RunTaskParams {
    std::string containerType;
    std::string sourceTest;
    std::string sourceRun;
};

struct Response {
    ErrorCode code;
    ErrorCode testErrorCode;
    std::string output;
};
}  // namespace watchman
