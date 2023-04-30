#include "common/logging.hpp"
#include "server/server.hpp"

#include <filesystem>
#include <string>

namespace {

std::string_view constexpr kConfig = "watchman_cpp_config.json";
std::string_view constexpr kEtcConfig = "/etc/watchman_cpp_config.json";

// we will find config at /etc and near the binary
std::string_view findConfig() {
    bool const existsNearBinary = std::filesystem::exists(kConfig);
    if (existsNearBinary) {
        watchman::Log::info("Config {} found near the binary", kConfig);
        return kConfig;
    }

    bool existsAtEtc = std::filesystem::exists(kEtcConfig);
    if (existsAtEtc) {
        watchman::Log::info("Config {} found at {}", kConfig, kEtcConfig);
        return kEtcConfig;
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
