#include "common/config.hpp"
#include "server/server.hpp"

int main() noexcept {
    auto config = watchman::getConfig();
    if (!config.has_value()) {
        return -1;
    }

    auto const threadPoolSize = config->threadPoolSize;
    watchman::Server server(std::move(*config));
    server.start(threadPoolSize);
    return 0;
}
