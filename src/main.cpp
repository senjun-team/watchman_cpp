#include "server/server.hpp"

static std::string const kConfig = std::string{CONFIG_DIR} + "config.json";

int main() {
    watchman::Server server(kDefaultHost, kConfig);
    server.start();
    return 0;
}
