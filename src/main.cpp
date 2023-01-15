#include "server/server.hpp"

int main() {
    watchman::Server server;
    server.start();
    return 0;
}
