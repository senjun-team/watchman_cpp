#include "service.hpp"

namespace watchman {

Response watchman::Service::runTask(watchman::RunTaskParams const & runTaskParams) {
    return {0, 0, "success"};
}
}  // namespace watchman
