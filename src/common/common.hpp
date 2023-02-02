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
    ErrorCode sourceCode{0};
    ErrorCode testsCode{0};
    std::string output;
};

// archiveName must have got suffix ".tar"
bool makeTar(std::string_view archiveName, std::string_view sourceCode);
}  // namespace watchman
