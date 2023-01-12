#pragma once

#include "../common/common.hpp"

namespace watchman {
class Service {
public:
    std::pair<Status, Text> runTask(RunTaskParams const & runTaskParams);

private:
};
}  // namespace watchman
