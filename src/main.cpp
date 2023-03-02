#include "server/server.hpp"

#include <string>

int main() noexcept {
    std::string const kConfig = "config.json";
    watchman::Server server(kDefaultHost, kConfig);
    server.start();
    return 0;
}
