#include "common/logging.hpp"
#include "server/server.hpp"

#include <filesystem>
#include <string>

namespace {

std::string const kConfig = "watchman_config.json";

// we will find config at /etc and near the binary
std::string findConfig() {
    bool const existsNearBinary = std::filesystem::exists(kConfig);
    if (existsNearBinary) {
        return kConfig;
    }

    std::string etcConfig = fmt::format("/etc/{}", kConfig);
    bool existsAtEtc = std::filesystem::exists(etcConfig);
    if (existsAtEtc) {
        return etcConfig;
    }

    return {};
}
}  // namespace

int main() noexcept {
    auto const configPath = findConfig();
    if (configPath.empty()) {
        watchman::Log::error("{} not found at /etc or near the binary", kConfig);
        return -1;
    }

    watchman::Server server(kDefaultHost, configPath);
    server.start();
    return 0;
}
