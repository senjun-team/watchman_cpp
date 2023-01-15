#pragma once

#include "../common/common.hpp"

namespace watchman {
class Service {
public:
    Response runTask(RunTaskParams const & runTaskParams);

private:
};
}  // namespace watchman
