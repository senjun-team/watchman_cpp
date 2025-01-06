#include "common/config.hpp"

#include "common/common.hpp"
#include "common/logging.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace watchman {

std::string_view constexpr kConfig = "watchman_cpp_config.json";
std::string_view constexpr kEtcConfig = "/etc/watchman_cpp_config.json";

std::string const kCpp = "cpp";
std::string const kGolang = "golang";
std::string const kHaskell = "haskell";
std::string const kPython = "python";
std::string const kRust = "rust";

std::unordered_map<std::string, Language> const kLanguagesMatch{
    {kCpp, Language::CPP},       {kGolang, Language::GO}, {kHaskell, Language::HASKELL},
    {kPython, Language::PYTHON}, {kRust, Language::RUST},
};

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

    watchman::Log::error("{} not found at /etc or near the binary", kConfig);
    return {};
}

template<typename Ptree>
void fillTable(Ptree const & taskType, ConfigContainerStorage & table, Api api) {
    if (!taskType.has_value()) {
        Log::error("Required field `{}` is absent", requiredApiField(api));
        std::terminate();
    }

    for (auto const & configLanguage : taskType.value()) {
        auto imageName = configLanguage.second.get_child_optional("image-name");
        if (!imageName.has_value()) {
            Log::error("Required field \'image-name\' is absent in {}", configLanguage.first);
            std::terminate();
        }

        auto const launched = configLanguage.second.get_child_optional("launched");
        if (!launched.has_value()) {
            Log::error("Required field \'launched\' is absent in {}", configLanguage.first);
            std::terminate();
        }

        auto const language = getLanguageFromString(configLanguage.first);
        table.insert({language,
                      {imageName.value().template get_value<std::string>(),
                       launched.value().template get_value<uint32_t>()}});
    }
}

template<typename Ptree>
Config fillConfig(Ptree const & root) {
    Config config;

    auto const & threadPoolSize = root.get_child_optional("thread-pool-size");
    config.threadPoolSize = threadPoolSize.value().template get_value<size_t>();

    if (config.threadPoolSize == 0) {
        config.threadPoolSize = getCpuCount();
    }

    auto const & maxContainersAmount = root.get_child_optional("max-containers-amount");
    if (!maxContainersAmount.has_value()) {
        Log::error("Required field \'max-containers-amount\' is absent");
        std::terminate();
    }
    config.maxContainersAmount = maxContainersAmount.value().template get_value<uint32_t>();

    fillTable(root.get_child_optional("courses"), config.courses, Api::ChapterTask);
    fillTable(root.get_child_optional("playground"), config.playgrounds, Api::Playground);
    fillTable(root.get_child_optional("practice"), config.practices, Api::Practice);
    return config;
}

Config readConfig(std::string_view configPath) {
    namespace pt = boost::property_tree;
    pt::ptree loadPtreeRoot;

    try {
        pt::read_json(configPath.data(), loadPtreeRoot);
        return fillConfig(loadPtreeRoot);
    } catch (std::exception const & error) {
        Log::error("Error while reading config file: {}", error.what());
        std::terminate();
    }
}

std::optional<Config> getConfig() {
    auto const path = findConfig();
    if (path.empty()) {
        return std::nullopt;
    }

    return readConfig(path);
}

Language getLanguageFromString(std::string const & language) {
    auto iLanguage = kLanguagesMatch.find(language);
    if (iLanguage == kLanguagesMatch.end()) {
        throw std::logic_error{
            fmt::format("constructTaskLauncher: unknown language: {}", language)};
    }

    return iLanguage->second;
}

}  // namespace watchman
