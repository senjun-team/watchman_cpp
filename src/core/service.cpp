#include "service.hpp"

namespace watchman {

std::pair<Status, Text> watchman::Service::runTask(watchman::RunTaskParams const & runTaskParams) {
    return {200, "success"};
}
}  // namespace watchman
