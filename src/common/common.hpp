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
    ErrorCode code{0};
    ErrorCode testErrorCode{0};
    std::string output;
};
}  // namespace watchman
