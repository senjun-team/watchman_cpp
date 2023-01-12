#pragma once

#include <string>

namespace watchman {

using Status = int32_t;
using Text = std::string;

struct RunTaskParams {
    std::string containerType;
    std::string sourceTest;
    std::string sourceRun;
};
}  // namespace watchman
