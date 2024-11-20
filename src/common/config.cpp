#include "common/config.hpp"

#include "common/logging.hpp"

#include <boost/property_tree/json_parser.hpp>

namespace watchman {

struct ConfgiHelper {
    std::unordered_map<Config::CodeLauncherType, Language> & table;
    std::string key;
    std::string suffix;
};

template<typename Ptree>
void fillTable(Ptree const & root, ConfgiHelper const & configHelper) {
    auto const & child = root.get_child_optional(configHelper.key);
    if (!child.has_value()) {
        Log::error("Required field `{}` is absent", configHelper.key);
        std::terminate();
    }

    for (auto const & field : child.value()) {
        auto imageName = field.second.get_child_optional("image-name");
        if (!imageName.has_value()) {
            Log::error("Required field \'image-name\' is absent in {}", field.first);
            std::terminate();
        }

        auto const launched = field.second.get_child_optional("launched");
        if (!launched.has_value()) {
            Log::error("Required field \'launched\' is absent in {}", field.first);
            std::terminate();
        }

        auto const & containerType = field.first;

        configHelper.table.insert({containerType + configHelper.suffix,
                                   {imageName.value().template get_value<std::string>(),
                                    launched.value().template get_value<uint32_t>()}});
    }
}

template<typename Ptree>
Config fillConfig(Ptree const & root) {
    Config config;

    auto const & threadPoolSize = root.get_child_optional("thread-pool-size");
    if (threadPoolSize.has_value()) {
        config.threadPoolSize = threadPoolSize.value().template get_value<size_t>();
    }

    auto const & maxContainersAmount = root.get_child_optional("max-containers-amount");
    if (!maxContainersAmount.has_value()) {
        Log::error("Required field \'max-containers-amount\' is absent");
        std::terminate();
    }
    config.maxContainersAmount = maxContainersAmount.value().template get_value<uint32_t>();

    // TODO remove suffixes, make it more intellectual
    fillTable(root, {config.languages, "courses", "_check"});
    fillTable(root, {config.playgrounds, "playground", "_playground"});
    fillTable(root, {config.practices, "practice", "_practice"});
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
}  // namespace watchman