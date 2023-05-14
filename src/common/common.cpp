#include "common.hpp"

#include "logging.hpp"
#include "tar_to_stream.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <thread>

namespace watchman {

struct StringContainers {
    static std::string_view constexpr python = "python";
    static std::string_view constexpr rust = "rust";
};

size_t getCpuCount() {
    constexpr size_t kDefCpuCount = 4;
    const size_t cpuCount = static_cast<size_t>(std::thread::hardware_concurrency());
    return cpuCount > 0 ? cpuCount : kDefCpuCount;
}

ContainerType getContainerType(std::string const & type) {
    if (type == StringContainers::python) {
        return ContainerType::Python;
    }

    if (type == StringContainers::rust) {
        return ContainerType::Rust;
    }

    return ContainerType::Unknown;
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

    auto const & languages = root.get_child_optional("languages");
    if (!languages.has_value()) {
        Log::error("Required field \'languages\' is absent");
        std::terminate();
    }

    for (auto const & language : languages.value()) {
        auto imageName = language.second.get_child_optional("image-name");
        if (!imageName.has_value()) {
            Log::error("Required field \'image-name\' is absent in {}", language.first);
            std::terminate();
        }

        auto const launched = language.second.get_child_optional("launched");
        if (!launched.has_value()) {
            Log::error("Required field \'launched\' is absent in {}", language.first);
            std::terminate();
        }

        auto const containerType = getContainerType(language.first);
        if (containerType == ContainerType::Unknown) {
            Log::error("Container type: \'{}\' is unknown", language.first);
            std::terminate();
        }

        config.languages.insert({containerType,
                                 {imageName.value().template get_value<std::string>(),
                                  launched.value().template get_value<uint32_t>()}});
    }

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

std::ostringstream makeTar(std::string const & sourceCode, std::string const & sourceTests) {
    std::ostringstream stream(std::ios::binary | std::ios::trunc);
    tar::tar_to_stream(stream, kFilenameTask, sourceCode.data(), sourceCode.size());
    tar::tar_to_stream(stream, kFilenameTaskTests, sourceTests.data(), sourceTests.size());
    tar::tar_to_stream_tail(stream);
    return stream;
}

LogDuration::LogDuration(std::string operation)
    : m_operation(std::move(operation))
    , m_start(std::chrono::system_clock::now()) {}

LogDuration::~LogDuration() {
    Log::info("Duration for {} is {} milliseconds", m_operation,
              std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                    - m_start)
                  .count());
}
};  // namespace watchman
